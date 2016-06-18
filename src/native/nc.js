'use strict';

(function(process) {

    console.log("Initializing Node-CEF...");

    this.global = this;
 
    // custom startup
    // NOTE:
    // - CMD switches are not supported yet.

    function startup() {
   
        var EventEmitter = NativeModule.require('events');

        process.__proto__ = Object.create(EventEmitter.prototype, {
            constructor: {
                value: process.constructor
            }
        });
        EventEmitter.call(process);

        startup.globalVariables();
        startup.globalTimeouts();

        startup.processVariables();
        // startup.processConfig();
        startup.processNextTick();
        startup.processKillAndExit();
        // startup.processSignalHandlers();
        
        var ncjs = {
            process : process,
            Buffer : NativeModule.require('buffer').Buffer
        };

        startup.runMain(ncjs); // or do your own main

        return ncjs;
    }

    // functions start here
    
    startup.globalVariables = function() {
        global.GLOBAL = global;
        global.root = global;
    };

    startup.globalTimeouts = function() {
        const timers = NativeModule.require('timers');
        global.setImmediate = timers.setImmediate;
        global.clearImmediate = timers.clearImmediate;
    };

    startup._lazyConstants = null;

    startup.lazyConstants = function() {
        if (!startup._lazyConstants)
            startup._lazyConstants = process.binding('constants');
        return startup._lazyConstants;
    };

    startup.processVariables = function() {
        process.domain = null;
        process._exiting = false;
        process._needImmediateCallback = false;
        // process.title
        Object.defineProperty(process, 'title', {
            get: function() { return document.title; } ,
            set: function(title) { document.title = title; },
            enumerable: true,
            configurable: true
        });
    };

    startup.processNextTick = function() {
        process.nextTick = nextTick;

        // just simply use setImmediate()
        function nextTick() {
            if (process._exiting)
                return;

            setImmediate.apply(window, arguments);
        }
    };

    startup.processKillAndExit = function() {
        // NOTE: process.exit() and process.abort() are forbidden by Node-CEF,
        // please use window.close() for similar purpose.
        process.exit = function(code) {
            throw new Error('Illegal invocation: exit(' + (code || 0) + ')');
        };

        process.abort = function() {
            throw new Error('Illegal invocation: abort()');
        };

        process.kill = function(pid, sig) {
            var err;
            
            if (pid != (pid | 0))
                throw new TypeError('invalid pid');

            // preserve null signal
            if (0 === sig) {
                err = process._kill(pid, 0);
            } else {
                sig = sig || 'SIGTERM';
                if (startup.lazyConstants()[sig] && sig.slice(0, 3) === 'SIG') {
                    err = process._kill(pid, startup.lazyConstants()[sig]);
                } else {
                    throw new Error('Unknown signal: ' + sig);
                }
            }

            if (err) {
                var errnoException = NativeModule.require('util')._errnoException;
                throw errnoException(err, 'kill');
            }

            return true;
        };
    };

    startup.runMain = function(ncjs) {
        var Module = NativeModule.require('module');
        var path = NativeModule.require('path');
        var module = new Module('.');
        var filename = path.resolve(process.argv[1]);
        // module.load()
        process.argv[1] = filename;
        module.filename = filename;
        module.exports = ncjs;
        module.paths = Module._nodeModulePaths(path.dirname(filename));
        var script = '_ncjs.__filename = __filename;\n' +
                     '_ncjs.exports = exports;\n' +
                     '_ncjs.module = module;\n' +
                     '_ncjs.__dirname = __dirname;\n' +
                     '_ncjs.require = require;\n';
        global._ncjs = ncjs;
        module._compile(script, filename);
        module.loaded = true;
        delete global._ncjs;
    }

    
    // build minimal module system

    // Neither CEF nor JavaScript have script compiling API,
    // so we just make a 'contextify' module replacement here.
    function bindContextifyModule() {
        function contextifyScript(code, options) {
            if (this.constructor != contextifyScript)
                throw Error('Must call vm.Script as a constructor.');

            var filename;
            if (options !== undefined) {
                if (typeof options === 'object') {
                    // offsets are not supported
                    if (options.lineOffset !== undefined && options.lineOffset != 0)
                        throw Error('"lineOffset" must be 0');
                    if (options.columnOffset !== undefined && options.columnOffset != 0)
                        throw Error('"columnOffset" must be 0');
                    // 'displayErrors' is ignored
                    filename = options.filename;
                } else if (typeof options === 'string') {
                    filename = options;
                } else {
                    throw TypeError('options must be an object');
                }
            }
            this.script = code;
            if (filename !== undefined)
                this.script += '\n//@ sourceURL=' + filename;
            // else chromium will use vmXX as filename.
        }

        contextifyScript.prototype.runInThisContext = function(options) {
            var displayErrors;
            var result;

            if (options !== undefined) {
                if (typeof options === 'object') {
                    displayErrors = options.displayErrors;
                    // 'timeout' is ignored
                } else {
                    throw TypeError('options must be an object');
                }
            }

            if (displayErrors !== false) {
                result = eval(this.script);
            } else {
                try {
                    result = eval(this.script);
                } catch (e) {}
            }

            return result;
        }

        var contextify = { ContextifyScript : contextifyScript };

        // add to binding cache
        process.binding._cache.contextify = contextify;

        return contextify;
    }

    var ContextifyModule = bindContextifyModule();

    function runInThisContext(code, options) {
        var script = new ContextifyModule.ContextifyScript(code, options);
        return script.runInThisContext();
    }

    function NativeModule(id) {
        this.filename = id + '.js';
        this.id = id;
        this.exports = {};
        this.loaded = false;
    }

    NativeModule._source = process.binding('natives');
    NativeModule._cache = {};

    NativeModule.require = function(id) {
        if (id == 'native_module')
            return NativeModule;

        var cached = NativeModule.getCached(id);
        
        if (cached)
            return cached.exports;

        if (!NativeModule.exists(id))
            throw new Error('No such native module ' + id);

        process.moduleLoadList.push('NativeModule ' + id);

        var nativeModule = new NativeModule(id);

        nativeModule.cache();
        nativeModule.compile();

        return nativeModule.exports;
    };
    
    NativeModule.getCached = function(id) {
        return NativeModule._cache[id];
    };

    NativeModule.exists = function(id) {
        return NativeModule._source.hasOwnProperty(id);
    };

    const EXPOSE_INTERNALS = process.execArgv.some(function(arg) {
        return arg.match(/^--expose[-_]internals$/);
    });        
    
    if (EXPOSE_INTERNALS) {
        NativeModule.nonInternalExists = NativeModule.exists;

        NativeModule.isInternal = function(id) {
            return false;
        };
    } else {
        NativeModule.nonInternalExists = function(id) {
            return NativeModule.exists(id) && !NativeModule.isInternal(id);
        };

        NativeModule.isInternal = function(id) {
            return id.startsWith('internal/');
        };
    }
    
    NativeModule.getSource = function(id) {
        return NativeModule._source[id];
    };

    NativeModule.wrap = function(script) {
        return NativeModule.wrapper[0] + script + NativeModule.wrapper[1];
    };

    NativeModule.wrapper = [
        '(function (exports, require, module, __filename, __dirname) { ',
        '\n});'
    ];

    NativeModule.prototype.compile = function() {
        var source = NativeModule.getSource(this.id);
        source = NativeModule.wrap(source);

        var fn = runInThisContext(source, {
            filename: this.filename,
            lineOffset: 0
        });
        fn(this.exports, NativeModule.require, this, this.filename);

        this.loaded = true;
    };

    NativeModule.prototype.cache = function() {
        NativeModule._cache[this.id] = this;
    };
    
    return startup();

});

//@ sourceURL=nc.js