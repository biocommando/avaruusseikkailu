const fs = require('fs')
const crypto = require('crypto')

const hash = data => {
        const a = new Int32Array(new Uint8Array(Buffer.from(crypto.createHash('sha256').update(data).digest('base64'), 'base64')).buffer)
        const b = new Int32Array([0])
        a.forEach(x => b[0] += x)
        return b[0]
    }

ids = {}

function getLinkCode(obj) {
    let name = obj.link_id
    if (!name)
        name = obj.data.name
    return '<' + obj.type + '.' + name + '>'
}

function addId(key, origKey) {
    const id = hash(key)
    if (!origKey) origKey = key
    if (Object.values(ids).includes(id)) {
        console.log('Id conflict (resolved automatically)', key)
        return addId(key + '_', origKey)
    }
    ids[origKey] = id
    return id
}

/*
    config items have the format:
    {
        type: enemy|collectable|weapon,
        data: {...}
    }
*/

const configFiles = []
fs
    .readdirSync('.')
    .filter(x => x.endsWith('.json'))
    .map(x => JSON.parse(fs.readFileSync(x).toString()))
    .forEach(x => {
        if (x.length) {
            x.forEach(y => y.data.id = addId(getLinkCode(y)))
            configFiles.push(...x)
        } else {
            x.data.id = addId(getLinkCode(x))
            configFiles.push(x)
        }
    })

const out = {
    enemy: '',
    collectable: '',
    weapon: ''
}

function link(id) {
    if (ids[id] === undefined)  throw "cannot link, id not found " + id
    return ids[id]
}

configFiles.forEach(x => {
    const add = key => {
        let data = String(x.data[key])
        key = key.replace(/<.*\..*>/, link)
        data = data.replace(/<.*\..*>/, link)
        out[x.type] += `${key}=${data}\n`
    }

    add('id')

    Object.keys(x.data)
        .filter(y => y !== 'id')
        .forEach(add)

    out[x.type] += '\n'
})

Object.keys(out).forEach(x => fs.writeFileSync('../config/profiles/' + x + '_profiles.ini', out[x]))