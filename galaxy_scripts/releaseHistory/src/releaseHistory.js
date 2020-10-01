var path = require('path');
var fs = require('fs');
var htmlPage = require('./htmlPage.js');
var jiraRequest = require('./jiraRequest.js');
global.familyProducts = {
    'GEX': {
        'prefix': '_gexpro_',
        'family': 'Examinator',
        'products': 'Examinator, Examinator-Pro',
        'productIds': ['GEX', 'GEX-Pro', 'Parsers'],
    },
    'YM': {
        'prefix': '_ym_',
        'family': 'Yield-Man',
        'products': 'Yieldman',
        'productIds': ['Yield-Man', 'Parsers'],
    },
    'PAT': {
        'prefix': '_pat_',
        'family': 'Patman',
        'products': 'Examinator-PAT, PAT-Man',
        'productIds': ['PAT-Man', 'Parsers'],
    },
    'GTM': {
        'prefix': '_gtm_',
        'family': 'GTM',
        'products': 'GTM',
        'productIds': ['GTL', 'GTM'],
    },
    'LM': {
        'prefix': '_License-Manager_',
        'family': 'License-Manager',
        'products': 'LM',
        'productIds': ['Licensing'],
    }
};

//values:
//GEX
//GEX-PAT
//GEX-Pro
//GTL
//GTM
//Licensing
//Parsers
//PAT-Man
//Yield-Man
//None Above 


var main = function () {
    opt = require('node-getopt').create([
        ['s', '', 'short option.'],
        ['', 'long', 'long option.'],
        ['S', 'short-with-arg=ARG', 'option with argument'],
        ['L', 'long-with-arg=ARG', 'long option with argument'],
        ['', 'color[=COLOR]', 'COLOR is optional'],
        ['u', 'multi-with-arg=ARG+', 'multiple option with argument'],
        ['', 'no-comment'],
        ['h', 'help', 'display this help'],
        ['v', 'version', 'show version'],
        ['n', 'noCheck', 'Do not check JIRA cases'],
        ['o', 'output=OUTPUT', 'set file output'],
        ['R', 'releases=RELEASES', 'set releases'],
        ['f', 'families=FAMILIES', 'set families'],
        ['', 'force', 'force Release History generation'],
        ['m', 'minor=MINOR', 'set minor version'],
        ['d', 'releaseDate=RELEASEDATE', 'set Date Of the release']
    ])              // create Getopt instance
            .bindHelp()     // bind option 'help' to default action
            .parseSystem(); // parse command line


    var releases = JSON.parse(opt.options.releases);
    var minor = opt.options.minor;
    var shortMinor = minor.replace('V','').replace('.','');
    var familyIds = JSON.parse(opt.options.families);
    var outputFile = opt.options.output;
    var noCheck = opt.options.noCheck;
    var releaseDate = opt.options.releaseDate;

    //var products = opt.options.products;
    var force = opt.options.force;

    var jiraData = [];
    var check = true;
    // get item from jira server
    for (var i = 0; i < releases.length; i++) {
        if (!noCheck) {
            check &= jiraRequest.checkJiraData(releases[i]);
        }
        jiraData[i] = jiraRequest.getJiraData(releases[i]);
    }
    
    if (!check && !force) {
        console.error("[ERROR] - Fix JIRA cases before building Release History.");
        process.exit(1);
    }

    //Create RH for version 
    for (var famIdx = 0; famIdx < familyIds.length; famIdx++) {
        var familyId = familyIds[famIdx];
        htmlPage.init(minor, familyId);
        htmlPage.appendToc(releases, familyId);
        for (var i = 0; i < jiraData.length; i++) {
            htmlPage.appendVersion(familyId, releases[i], jiraData[i]);
        }
        var page = htmlPage.end();
        if (outputFile) {
            var myFile = path.dirname(outputFile) + '/' + familyProducts[familyId].prefix + path.basename(outputFile);
            fs.writeFileSync(myFile, page);
            console.log('File created: ' + myFile);
        } else {
            console.log(page);
        }
        //update Global release history
        var prefix = familyProducts[familyId].prefix;

        //update global version if required.
        //check produc_history.htm if exist
        var histoFile = path.dirname(outputFile) + '/' + prefix + 'history.htm';
        if (fileExists(histoFile)) { 

            //get content & check if produc_history.htm already contain the minor version
            var htmlSource =  fs.readFileSync(histoFile, "utf-8");
            if(htmlSource.indexOf(">"+minor+"<") < 0) {

                //get template and replace info
                var newLineVersion = fs.readFileSync('./resources/newVersion.txt', "utf-8");
                newLineVersion = newLineVersion.replace(/___MINOR___/g,minor);
                newLineVersion = newLineVersion.replace(/___PREFIX___/g,prefix);
                newLineVersion = newLineVersion.replace(/___SHORTMINOR___/g,shortMinor);
                newLineVersion = newLineVersion.replace(/___RELEASEDATE___/g,releaseDate);
                
                //update historyFile
                htmlSource= htmlSource.replace(/      <\/tbody><\/table>/, newLineVersion);
                fs.writeFileSync(histoFile, htmlSource);
                console.log('File updated: ' + histoFile);
            }
        }
    }
    

    
    
}

main();


function fileExists(filePath)
{
    try
    {
        return fs.statSync(filePath).isFile();
    }
    catch (err)
    {
        return false;
    }
}
