OnceDB is a high-performance full-text search database based on Redis. OnceDB can greatly improve the performance of data queries through auxiliary indexes. Like SQL databases, you don't need to care about the details of index creation. OnceDB has very good performance on low-end ARM devices.

OnceDB uses operators to dynamically define indexes. OnceDB do not changing the data storage structure of Redis. You can use existing Redis tools to view and manage the data in OnceDB.

# Document

https://oncedb.com/wiki/view/oncedb-server

# Driver

Node.js:

You can use the following code to update and query the data:

```
const util    = require('util')
const oncedb  = require('oncedb')()

const update  = util.promisify(oncedb.update).bind(oncedb)
const select  = util.promisify(oncedb.select).bind(oncedb)

//Defining the schema
oncedb.schema('user', {
    username  : 'id'
  , password  : ''
  , title     : 'index'
  , skills    : 'keywords'
});

(async () => {
  // update data
  await upsert('user', { username: 'dota', password: '123456', title: 'SDEI', skills: 'java,go,c' })
  // query data
  let rows = await select('user', { skills: 'c' })

  console.log('rows.count', rows.count)
  console.log(rows)
})();

```
Result

```
rows.count 1
[
  {
    _key: 'user:dota',
    skills: [ 'java', 'go', 'c' ],
    username: 'dota',
    password: '123456',
    title: 'SDEI'
  }
]
```


# Release

[Release Page](https://github.com/OnceDoc/OnceDB/releases)

