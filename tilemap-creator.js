function createTilemap(map, tileFile, outputFileName) {
    map.forEach(x => {
        x.obstacleType = x.type === 'props_1' ? 'x' : '.'
        x.props = Number(x.type.replace('props_', '').replace('m', '-'))
    })

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
        result.push({ x: point.x, y: point.y, sx, sy, description, props: point.props })
    }

    function randomWall() {
        const sx = Math.floor(Math.random() * (6 - 1e-6))
        const sy = Math.floor(Math.random() * (2 - 1e-6) + 1)
        if (sx === 4 && sy === 2)
            return randomWall()
        return { sx, sy, description: "generic wall", props: 1 }
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
        // If the logic above failed to place a wall, place a random wall
        if (point.props === 1)
            tile(point, randomWall())
        point.objectIds.forEach(obj => {
            result.push({ description: 'object', x: point.x, y: point.y, type: obj.id, objectType: obj.type})
        })
        if (point.props === 0) {
            let sx = Math.floor(Math.random() * (5 - 1e-6))
            let sy = Math.floor(Math.random() * (2 - 1e-6)) + 6
            result.push({ x: point.x, y: point.y, sx, sy, description: 'background', props: point.props })
        }
        else if (point.props === 2) {
            result.push({
                x: point.x, y: point.y, sx: 5, sy: 6, description: 'hazard',
                a: 1, a_flen: 10, a_fnum: 3, props: point.props
            })
        } else if ((point.props > 2 && point.props < 10) || (point.props < -2 && point.props > -10)) {
            result.push({
                x: point.x, y: point.y, sx: 5 + (point.props > 0 ? 0 : 1), sy: 7, description: 'flippable',
                props: point.props
            })
        }
    })

    result.sort((a, b) => {
        if (a.objectType === b.objectType) return 0
        if (a.objectType === 'building') return -1
        if (b.objectType === 'building') return 1
        return 0
    })

    let output = `sprite_sheet=${tileFile}\r\nw=32\r\nh=32\r\n`

    let prev = { x: -1, sx: -1, sy: -1, y: -1, props: -1, a_fnum: -1, a_flen: -1, a: 0, props: undefined }
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
        if (point.a) {
            if (point.a_fnum !== prev.a_fnum)
                output += `a_fnum=${point.a_fnum}\r\n`
            if (point.a_flen !== prev.a_flen)
                output += `a_flen=${point.a_flen}\r\n`
        }
        if (point.a !== prev.a)
            output += `a=${point.a ? point.a : 0}\r\n`
        if (point.props !== prev.props)
            output += `props=${point.props}\r\n`
        prev = { ...point }
        output += 'set tile\r\n'
    })

    //console.log(output)

    const fs = require('fs')

    fs.writeFileSync(outputFileName, output)

}

module.exports = { createTilemap }