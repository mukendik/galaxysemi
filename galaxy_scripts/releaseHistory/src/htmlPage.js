var fs = require('fs');
var escape = require('escape-html');

var page;

var init = function (minor, familyId) {
    var header = fs.readFileSync('./resources/header.txt', "utf-8");
    header = header.replace(/__MINOR__/g, minor);
    header = header.replace(/__FAMILY__/g, familyProducts[familyId].family);
    header = header.replace(/__PRODUCTS__/g, familyProducts[familyId].products);
    page = header;
}

var end = function () {
    var footer = fs.readFileSync('./resources/footer.txt', "utf-8");
    page += footer;
    return page;
}

var appendToc = function (versions, familyId) {
    var toc = fs.readFileSync('./resources/tocHeader.txt', "utf-8");
    toc = toc.replace(/___PREFIX___/, familyProducts[familyId].prefix);
    for (var i = 0; i < versions.length; i++) {
        toc += '                            <a href="#' + versions[i] + '">' + versions[i] + '</a><br>\n\
';
    }
    toc += fs.readFileSync('./resources/tocFooter.txt', "utf-8");
    page += toc;
};


var appendVersion = function (familyId, version, jiraData) {
    var versionRHTitle = fs.readFileSync('./resources/versionRHTitle.txt', "utf-8");
    versionRHTitle = versionRHTitle.replace(/__VERSION__/g, version);

    page += versionRHTitle;
    page += fs.readFileSync('./resources/versionRHHeader01.txt', "utf-8");

    var versionRHEnhancement, versionRHBug, versionRHUndef;
    var versionRHEnhancementEmpty = true;
    var versionRHBugEmpty = true;
    var versionRHEnhancement = fs.readFileSync('./resources/versionRHHeader02.txt', "utf-8");
    versionRHEnhancement = versionRHEnhancement.replace(/__ISSUE_TYPE__/g, "Enhancements");
    var versionRHBug = fs.readFileSync('./resources/versionRHHeader02.txt', "utf-8");
    versionRHBug = versionRHBug.replace(/__ISSUE_TYPE__/g, "Bug fixes");
    // Enhancements / Bug fixes
    for (var i = 0; i < jiraData.issues.length; i++) {
        var match = false;
        for (var val = 0; jiraData.issues[i].fields.customfield_11300 && val < jiraData.issues[i].fields.customfield_11300.length; val++) {
            var galaxyProduct = jiraData.issues[i].fields.customfield_11300[val].value;
            if (familyProducts[familyId].productIds.indexOf(galaxyProduct) > -1) {
                match = true;
//                console.error('productIds: ' + familyProducts[familyId].productIds);
//                console.error('galaxyProduct: ' + JSON.stringify(galaxyProduct));
            }
        }
        if (!match) {
            continue;
        }
        versionRHUndef = '           <tr>\n\
                <td style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000" height="38" align="left" valign=top'
                + ' SDVAL="' + jiraData.issues[i].key + '"'
                + ' SDNUM="' + jiraData.issues[i].id + '"'
                + '><font face="Verdana">' + jiraData.issues[i].key + '</font></td>\n\
                <td style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000" align="left" valign=top><font face="Verdana">'
                + jiraData.issues[i].fields.customfield_10900 + '</font></td>\n\
                <td style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; border-right: 1px solid #000000" align="left" valign=top><font face="Verdana">'
//                + ((jiraData.issues[i].fields.customfield_10901 != null)?jiraData.issues[i].fields.customfield_10901.replace(/(\r\n|\n|\r)/g,"\n<br/>\n"):"") + '</font></td>\n\
                + ((jiraData.issues[i].fields.customfield_10901 != null)?escape(jiraData.issues[i].fields.customfield_10901).replace(/(\r\n|\n|\r)/g,"\n<br/>\n"):"") + '</font></td>\n\
            </tr>\n\
';
//        if (jiraData.issues[i].fields.issuetype.name === 'Story') {
        if (jiraData.issues[i].fields.issuetype.name === 'Story' || jiraData.issues[i].fields.issuetype.name === 'Epic') {
            versionRHEnhancement += versionRHUndef;
            versionRHEnhancementEmpty = false;
        } else if (jiraData.issues[i].fields.issuetype.name === 'Bug') {
            versionRHBug += versionRHUndef;
            versionRHBugEmpty = false;
        }
    }
    if (!versionRHEnhancementEmpty) {
        page += versionRHEnhancement;
    }
    if (!versionRHBugEmpty) {
        page += versionRHBug;
    }
    page += fs.readFileSync('./resources/versionRHFooter.txt', "utf-8");
}

exports.init = init;
exports.end = end;
exports.appendToc = appendToc;
exports.appendVersion = appendVersion;;
