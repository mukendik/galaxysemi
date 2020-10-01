var https = require('https');

var body = JSON.stringify({
    'jql': 'fixVersion = V7.5.0-gamma AND type != Sub-task AND ("Include in Release History?" is EMPTY OR "Include in Release History?" = Yes)',
    'fields': ['id', 'key', 'summary', 'description']
});

//The url we want is `www.nodejitsu.com:1337/`
var options = {
    hostname: 'galaxysemi.atlassian.net',
    port: 443,
    path: '/rest/api/2/search',
    method: 'POST',
    headers: {
        'Content-Type': 'application/json',
        'Authorization': 'Basic c2ViYXN0aWVuLmJlYXVncmFuZDphbHRpbmdsaTYzMA==',
        'Content-Length': Buffer.byteLength(body)
    }
};

//curl -H "Authorization: Basic c2ViYXN0aWVuLmJlYXVncmFuZDphbHRpbmdsaTYzMA" \
//-X POST -H "Content-Type: application/json" \
//--data '{"jql":"fixVersion = V7.5.0-gamma AND type != Sub-task AND (\"Include in Release History?\" is EMPTY OR \"Include in Release History?\" = Yes)","fields":["id","key"]}' \
//"https://galaxysemi.atlassian.net/rest/api/2/search"

var req = https.request(options, function (res) {
    console.log("statusCode: ", res.statusCode);
    console.log("headers: ", res.headers);
    res.on('data', function (d) {
        process.stdout.write(d);
    });
});
req.end(body);
req.on('error', function (e) {
    console.error(e);
});

var main = function () {
    opt = require('node-getopt').create([
        ['s', '', 'short option.'],
        ['', 'long', 'long option.'],
        ['S', 'short-with-arg=ARG', 'option with argument'],
        ['L', 'long-with-arg=ARG', 'long option with argument'],
        ['', 'color[=COLOR]', 'COLOR is optional'],
        ['m', 'multi-with-arg=ARG+', 'multiple option with argument'],
        ['', 'no-comment'],
        ['h', 'help', 'display this help'],
        ['v', 'version', 'show version']
    ])              // create Getopt instance
            .bindHelp()     // bind option 'help' to default action
            .parseSystem(); // parse command line

    console.info(opt);
}

main();