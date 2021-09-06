const fs = require('fs')

const lines = fs.readFileSync(process.argv[2]).toString().split('\n')
const outLines = []
let add = false
lines.forEach(x => {
    if (add) {
        outLines.push(x)
        add = !x.includes('SYNTH_DATA_END')
    } else {
        add = x.includes('SYNTH_DATA_START')
    }
})

fs.writeFileSync(process.argv[2].replace('.flp', '.mid_meta.ini'), outLines.join('\n') + '\n')