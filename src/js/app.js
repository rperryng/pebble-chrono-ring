var Clay = require('clay');
var clayConfig = require('config.json');
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function(e) {
  console.log('Javascript app ready and running!');
});
