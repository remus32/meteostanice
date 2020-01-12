import * as assert from 'assert'
import * as express from 'express'

const port = +(process.env.PORT || '')
assert(port > 0)
const app = express()

app.listen(port, () => {
  console.log(`Listening on port ${port}`)
})
