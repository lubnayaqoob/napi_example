'use strict'

const addon = require('bindings')('addon')

const cb = function (err, data) {
    console.log('This is the JS callback')
    if (err) {
        console.error("SOmething bad happened")
        console.error(err)
    } else {
        console.log("len: ", data.len)
        console.log("id: ", data.id)
        // Iterate over the buffer
        for (const el of data.buffer) {
            console.log(el)
        }
    }   
}

addon.NapiCanrcvFunction("vcan0", cb)



