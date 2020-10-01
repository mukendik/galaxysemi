var http = require('http');
var https = require('https');
var request = require('sync-request');
var netrc = require('netrc');

var checkJiraData = function(version) {
    var check = true;
    var jql = 'fixVersion = ' + version + ' AND type != Sub-task AND ("Include in Release History?" is EMPTY OR "Include in Release History?" = Yes)' + ' AND ("Validated Version/s" != ' + version + ' OR "Release History Summary" is EMPTY OR "Release History Description" is EMPTY OR "Galaxy Products" is EMPTY)';
    //    console.log("jql to check JIRA data: " + jql);
    var myNetrc = netrc();
    var res = request('POST', 'https://galaxysemi.atlassian.net/rest/api/2/search', {
        headers: {
            'Content-Type': 'application/json',
            'Authorization': '' + myNetrc['galaxysemi.atlassian.net'].login + ' ' + myNetrc['galaxysemi.atlassian.net'].password
        },
        json: {
            'jql': jql,
            'fields': ['id', 'key', 'summary', 'description']
        }
    });
    var result = JSON.parse(res.getBody('utf8'));
    if (result.issues.length > 0) {
        check = false;
        console.error('Version ' + version + " KO!");
        for (var i = 0; i < result.issues.length; i++) {
            console.error('- ' + result.issues[i].key + " KO!");
        }
    }
    return check;
    //    console.log(JSON.parse(res.getBody('utf8')));
}

var getJiraData = function(version) {
    var jql = '"Validated Version/s" = ' + version + ' AND type != Sub-task AND ("Include in Release History?" is EMPTY OR "Include in Release History?" = Yes)';
    //    console.log("jql to get JIRA data: " + jql);
    var res = request('POST', 'https://galaxysemi.atlassian.net/rest/api/2/search', {
        headers: {
            'Content-Type': 'application/json',
            'Authorization': 'Basic c2ViYXN0aWVuLmJlYXVncmFuZDphbHRpbmdsaTYzMA=='
        },
        json: {
            'jql': jql,
            'fields': ['id', 'key', 'summary', 'description', 'issuetype', 'customfield_11300', 'customfield_10900', 'customfield_10901']
        }
    });
    var result = JSON.parse(res.getBody('utf8'));
    //    console.log("XXX: " + JSON.stringify(result.issues));
    return result;
}

exports.checkJiraData = checkJiraData;
exports.getJiraData = getJiraData;
