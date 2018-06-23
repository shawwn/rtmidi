Object.assign(module.exports, require(`./build/${process.env.RTMIDI_DEBUG ? 'Debug' : 'Release'}/rtmidi.node`));
