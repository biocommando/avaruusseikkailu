const map = require('./' + process.argv[2]).map(x => ({ ...x, type: x.color[0] ? '.' : 'x' }))

function checkPattern(point, pattern) {
    let i = 0
    for (let y = point.y - 1; y <= point.y + 1; y++) {
        for (let x = point.x - 1; x <= point.x + 1; x++) {
            if (pattern[i] !== '') {
                const point2 = map.find(p => p.x === x && p.y === y)
                if (!point2 && pattern[i] !== 'x')
                    return false
                if (point2 && point2.type !== pattern[i])
                    return false
            }
            i++
        }
    }
    return true
}

function checkPatternCb(point, pattern, callback) {
    if (checkPattern(point, pattern)) callback()
}

const X = 'x'
const _ = '.'
const $ = ''

const result = []

let blockTileAdd

function tile(point, tileType) {
    if (blockTileAdd)
        return
    blockTileAdd = true
    const { sx, sy, description } = tileType
    result.push({ x: point.x, y: point.y, sx, sy, description })
}

function randomWall() {
    const sx = Math.floor(Math.random() * 6)
    const sy = Math.floor(Math.random() * 2 + 1)
    if (sx === 4 && sy === 2)
        return randomWall()
    return { sx, sy, description: "generic wall" }
}

map.forEach((point, i) => {
    blockTileAdd = false

    checkPatternCb(point,
        [
            $, X, $,
            X, X, X,
            _, X, $
        ], () => tile(point, { sx: 3, sy: 4, description: "border-angle" }))
    checkPatternCb(point,
        [
            $, X, $,
            X, X, X,
            $, X, _
        ], () => tile(point, { sx: 4, sy: 4, description: "border-angle" }))
    checkPatternCb(point,
        [
            $, X, _,
            X, X, X,
            $, X, $
        ], () => tile(point, { sx: 4, sy: 3, description: "border-angle" }))
    checkPatternCb(point,
        [
            _, X, $,
            X, X, X,
            $, X, $
        ], () => tile(point, { sx: 3, sy: 3, description: "border-angle" }))

    checkPatternCb(point,
        [
            $, X, $,
            X, X, X,
            $, X, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, _, $,
            X, X, _,
            $, _, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, _, $,
            _, X, _,
            $, X, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, _, $,
            _, X, X,
            $, _, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, X, $,
            _, X, _,
            $, _, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, _, $,
            _, X, _,
            $, _, $
        ], () => tile(point, randomWall()))
    checkPatternCb(point,
        [
            $, X, $,
            _, X, X,
            $, X, $
        ], () => tile(point, { sx: 0, sy: 4, description: "border-vert" }))
    checkPatternCb(point,
        [
            $, X, $,
            X, X, _,
            $, X, $
        ], () => tile(point, { sx: 2, sy: 4, description: "border-vert" }))
    checkPatternCb(point,
        [
            $, X, $,
            _, X, _,
            $, X, $
        ], () => tile(point, { sx: 1, sy: 0, description: "border-vert-narrow" }))
    checkPatternCb(point,
        [
            $, _, $,
            X, X, X,
            $, _, $
        ], () => tile(point, { sx: 2, sy: 0, description: "border-hor-narrow" }))
    checkPatternCb(point,
        [
            $, X, $,
            X, X, X,
            $, _, $
        ], () => tile(point, { sx: 1, sy: 5, description: "border-hor" }))
    checkPatternCb(point,
        [
            $, _, $,
            X, X, X,
            $, X, $
        ], () => tile(point, { sx: 1, sy: 3, description: "border-hor" }))
    checkPatternCb(point,
        [
            $, _, $,
            _, X, X,
            $, X, X
        ], () => tile(point, { sx: 0, sy: 3, description: "border-angle" }))
    checkPatternCb(point,
        [
            $, X, X,
            _, X, X,
            $, _, $
        ], () => tile(point, { sx: 0, sy: 5, description: "border-angle" }))
    checkPatternCb(point,
        [
            X, X, $,
            X, X, _,
            $, _, $
        ], () => tile(point, { sx: 2, sy: 5, description: "border-angle" }))
    checkPatternCb(point,
        [
            $, _, $,
            X, X, _,
            X, X, $
        ], () => tile(point, { sx: 2, sy: 3, description: "border-angle" }))
    if (point.type === '.') {
        if (point.color.join('-') === '255-0-0')
            result.push({ description: 'object', x: point.x, y: point.y, type: 1 })
        else if (point.color.join('-') === '255-255-0')
            result.push({ description: 'object', x: point.x, y: point.y, type: 2 })
        else if (point.color.join('-') === '255-127-0')
            result.push({ description: 'object', x: point.x, y: point.y, type: 3 })
        else if (point.color.join('-') === '255-0-255')
            result.push({ description: 'object', x: point.x, y: point.y, type: 0 })
    }
})

//console.log(result)
let output = `sprite_sheet=sprites/tile01.png\r\nw=32\r\nh=32\r\n`

result.forEach(point => {
    if (point.description === 'object') {
        output += `obj_x=${point.x * 32}\r\n`
        output += `obj_y=${point.y * 32}\r\n`
        output += `obj_type=${point.type}\r\n`
        output += `set object\r\n`
        return;
    }
    output += `x=${point.x * 32}\r\n`
    output += `y=${point.y * 32}\r\n`
    output += `sx=${point.sx * 32}\r\n`
    output += `sy=${point.sy * 32}\r\n`
    output += `props=${point.description !== 'background' ? 1 : 0}\r\n`
    output += 'set tile\r\n'
})

//console.log(output)

const fs = require('fs')

fs.writeFileSync('config/' + process.argv[2] + '.ini', output)