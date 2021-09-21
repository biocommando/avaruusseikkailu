function createTilemap(map, tileFile, outputFileName) {
    map.forEach(x => x.obstacleType = x.type === 'wall' ? 'x' : '.')

    function checkPattern(point, pattern) {
        let i = 0
        for (let y = point.y - 1; y <= point.y + 1; y++) {
            for (let x = point.x - 1; x <= point.x + 1; x++) {
                if (pattern[i] !== '') {
                    const point2 = map.find(p => p.x === x && p.y === y)
                    if (!point2 && pattern[i] !== 'x')
                        return false
                    if (point2 && point2.obstacleType !== pattern[i])
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
        const sx = Math.floor(Math.random() * (6 - 1e-6))
        const sy = Math.floor(Math.random() * (2 - 1e-6) + 1)
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
        /*if (point.type === '.') {
            if (point.color.join('-') === '255-0-0')
                result.push({ description: 'object', x: point.x, y: point.y, type: 1 })
            else if (point.color.join('-') === '255-255-0')
                result.push({ description: 'object', x: point.x, y: point.y, type: 2 })
            else if (point.color.join('-') === '255-127-0')
                result.push({ description: 'object', x: point.x, y: point.y, type: 3 })
            else if (point.color.join('-') === '255-0-255')
                result.push({ description: 'object', x: point.x, y: point.y, type: 0 })
            else if (point.color.join('-') === '127-127-127')
                result.push({ description: 'object', x: point.x, y: point.y, type: 101 })
            else if (point.color.join('-') === '200-200-200')
                result.push({ description: 'object', x: point.x, y: point.y, type: 102 })
            else if (point.color.join('-') === '220-220-220')
                result.push({ description: 'object', x: point.x, y: point.y, type: 103 })
            else if (point.color.join('-') === '100-100-100')
                result.push({ description: 'object', x: point.x, y: point.y, type: 104 })
        }*/
        point.objectIds.forEach(obj => {
            result.push({ description: 'object', x: point.x, y: point.y, type: obj })
        })
        if (point.type === '')
        {
            let sx = Math.floor(Math.random() * (5 - 1e-6))
            let sy = Math.floor(Math.random() * (2 - 1e-6)) + 6
            result.push({ x: point.x, y: point.y, sx, sy, description: 'background' })
        }
        else if (point.type === 'hazard')
        {
            result.push({ x: point.x, y: point.y, sx: 6, sy: 1, description: 'hazard' })
        }
    })

    let output = `sprite_sheet=${tileFile}\r\nw=32\r\nh=32\r\n`

    let prev = { x: -1, sx: -1, sy: -1, y: -1, props: -1 }
    result.forEach(point => {
        if (point.description === 'object') {
            output += `obj_x=${point.x * 32}\r\n`
            output += `obj_y=${point.y * 32}\r\n`
            output += `obj_type=${point.type}\r\n`
            output += `set object\r\n`
            return;
        }
        if (point.x !== prev.x)
            output += `x=${point.x * 32}\r\n`
        if (point.y !== prev.y)
            output += `y=${point.y * 32}\r\n`

        if (point.sx !== prev.sx)
            output += `sx=${point.sx * 32}\r\n`
        if (point.sy !== prev.sy)
            output += `sy=${point.sy * 32}\r\n`
        let props = 1
        if (point.description === 'background')
            props = 0
        if (point.description === 'hazard')
            props = 2
        if (props !== prev.props)
            output += `props=${props}\r\n`
        prev = { ...point, props }
        output += 'set tile\r\n'
    })

    //console.log(output)

    const fs = require('fs')

    fs.writeFileSync(outputFileName, output)

}

module.exports = { createTilemap }