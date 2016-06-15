// MODIFIED: use browser implementation
'use strict';

exports.setTimeout   = setTimeout.bind(window);
exports.clearTimeout = clearTimeout.bind(window);

exports.setInterval   = setInterval.bind(window);
exports.clearInterval = clearInterval.bind(window);

exports.setImmediate = function(callback) {
    var args = [callback, 0];

    for (var i = 1; i < arguments.length; i++)
            args.push(arguments[i]);

    return setTimeout.apply(null, args);
}
exports.clearImmediate = clearTimeout.bind(window);
