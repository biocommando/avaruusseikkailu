const http = require('http')
const fs = require('fs')
const { createTilemap } = require('./tilemap-creator')
const { spawn } = require('child_process');

const mimeTypeMap = { '.aac': 'audio/aac', '.abw': 'application/x-abiword', '.arc': 'application/x-freearc', '.avi': 'video/x-msvideo', '.azw': 'application/vnd.amazon.ebook', '.bin': 'application/octet-stream', '.bmp': 'image/bmp', '.bz': 'application/x-bzip', '.bz2': 'application/x-bzip2', '.csh': 'application/x-csh', '.css': 'text/css', '.csv': 'text/csv', '.doc': 'application/msword', '.docx': 'application/vnd.openxmlformats-officedocument.wordprocessingml.document', '.eot': 'application/vnd.ms-fontobject', '.epub': 'application/epub+zip', '.gz': 'application/gzip', '.gif': 'image/gif', '.htm': 'text/html', '.html': 'text/html', '.ico': 'image/vnd.microsoft.icon', '.ics': 'text/calendar', '.jar': 'application/java-archive', '.jpeg': 'image/jpeg', '.jpg': 'image/jpeg', '.js': 'text/javascript', '.json': 'application/json', '.jsonld': 'application/ld+json', '.mid': 'audio/midi audio/x-midi', '.midi': 'audio/midi audio/x-midi', '.mjs': 'text/javascript', '.mp3': 'audio/mpeg', '.mpeg': 'video/mpeg', '.mpkg': 'application/vnd.apple.installer+xml', '.odp': 'application/vnd.oasis.opendocument.presentation', '.ods': 'application/vnd.oasis.opendocument.spreadsheet', '.odt': 'application/vnd.oasis.opendocument.text', '.oga': 'audio/ogg', '.ogv': 'video/ogg', '.ogx': 'application/ogg', '.opus': 'audio/opus', '.otf': 'font/otf', '.png': 'image/png', '.pdf': 'application/pdf', '.php': 'application/php', '.ppt': 'application/vnd.ms-powerpoint', '.pptx': 'application/vnd.openxmlformats-officedocument.presentationml.presentation', '.rar': 'application/x-rar-compressed', '.rtf': 'application/rtf', '.sh': 'application/x-sh', '.svg': 'image/svg+xml', '.swf': 'application/x-shockwave-flash', '.tar': 'application/x-tar', '.tif': 'image/tiff', '.tiff': 'image/tiff', '.ts': 'video/mp2t', '.ttf': 'font/ttf', '.txt': 'text/plain', '.vsd': 'application/vnd.visio', '.wav': 'audio/wav', '.weba': 'audio/webm', '.webm': 'video/webm', '.webp': 'image/webp', '.woff': 'font/woff', '.woff2': 'font/woff2', '.xhtml': 'application/xhtml+xml', '.xls': 'application/vnd.ms-excel', '.xlsx': 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet', '.xml': 'application/xml', '.xul': 'application/vnd.mozilla.xul+xml', '.zip': 'application/zip', '.3gp': 'video/3gpp', '.3g2': 'video/3gpp2', '.7z': 'application/x-7z-compressed', }

http.createServer((req, res) => {
    try {
        let url = req.url
        url = url.split('?')[0]
        console.log('requested', url)
        if (url.split('/')[1] === 'api') {
            const cmd = url.split('/')[2]
            const param = url.split('/')[3]
            if (cmd === 'object-types') {
                const objectTypes = []
                let objectType
                const mapObjectProfile = x => {
                    if (x[0] === 'id') {
                        objectType = { id: Number(x[1]), name: '??', type: '??' }
                        objectTypes.push(objectType)
                    } else if (x[0] === 'name') {
                        objectType.name = x[1]
                    } else if (x[0] === 'type') {
                        objectType.type = x[1]
                    }
                }
                fs.readFileSync('config/profiles/collectable_profiles.ini').toString()
                    .split(/\r?\n/)
                    .map(x => x.split('='))
                    .forEach(mapObjectProfile)
                fs.readFileSync('config/profiles/enemy_profiles.ini').toString()
                    .split(/\r?\n/)
                    .map(x => x.split('='))
                    .forEach(mapObjectProfile)
                res.writeHead(200, { 'Content-Type': 'application/json' })
                res.end(JSON.stringify(objectTypes))
                return
            } else if (cmd === 'save') {
                let body = ''
                req.on('data', data => body += data)
                req.on('end', () => {
                    fs.writeFileSync('map-projects/' + param, body)
                    res.end('ok')
                })
                return
            } else if (cmd === 'export') {
                let body = ''
                req.on('data', data => body += data)
                req.on('end', () => {
                    const obj = JSON.parse(body)
                    createTilemap(obj.data, obj.spriteSheet, 'config/map/' + param)
                    let missionConfigFragment = [
                        'map=' + 'config/map/' + param,
                        'name=' + obj.name,
                        'num=' + obj.num
                    ]
                    if (obj.buyAllowFlags)
                        missionConfigFragment.push('buy_allow_flags=' + obj.buyAllowFlags)
                    obj.goals.forEach(goal => {
                        missionConfigFragment.push('goal_type=' + goal.type)
                        missionConfigFragment.push('goal_value=' + goal.value)
                        missionConfigFragment.push('set goal')
                    })
                    obj.triggers.forEach(trig => {
                        missionConfigFragment.push('trigger_id=' + trig.id)
                        missionConfigFragment.push('trigger_type=' + trig.type)
                        missionConfigFragment.push('trigger_text=' + trig.text)
                        missionConfigFragment.push('set trigger')
                    })
                    missionConfigFragment.push('end')
                    missionConfigFragment.push('')
                    fs.writeFileSync('config/map/config.' + param, missionConfigFragment.join('\n'))
                    res.end('ok')
                })
                return
            } else if (cmd === 'list-proj') {
                res.writeHead(200, { 'Content-Type': 'application/json' })
                res.end(JSON.stringify(fs.readdirSync('map-projects').filter(x => x.endsWith('.json'))))
                return
            } else if (cmd === 'testplay') {
                const params = ['-testplay_mission' , 'config/map/config.' + param]
                console.log('Launching game with params', params)
                spawn('avaruusseikkailu.exe', params)
                return
            }
        }
        if (url === '/') url = '/mapedit.html'
        const extension = '.' + url.replace(/.+\./, '')
        let mimeType = mimeTypeMap[extension]
        if (!mimeType) mimeType = 'text/plain'
        res.writeHead(200, { 'Content-Type': mimeType })
        res.end(fs.readFileSync(url.substr(1)))
    } catch (e) {
        console.log('error', e)
        res.end()
    }
}).listen(3000)

console.log('serving at port 3000')