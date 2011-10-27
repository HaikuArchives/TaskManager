set_stylesheet();

// Tries to find the sting "pat" inside the sting "str".
// Returns: Index of the first occurance, -1 if not found.
// I use this function, because string::search isn't working
// correctly in opera 4!
function search(str, pat)
{
    var patpos=0;

    for(var i=0 ; i<str.length ; i++) {
	if(str.charAt(i) == pat.charAt(patpos)) {
	    patpos++;

	    if(patpos >= pat.length)
		return i-pat.length+1;
	} else {
	    patpos = 0;
	}
    }

    return -1;
}

function set_stylesheet()
{
    var css = "opera.css";

    if (navigator.appName == "Microsoft Internet Explorer") {
	css = "ie.css";
    } else if(navigator.appName == "Netscape") {
	if(search(navigator.userAgent, "Opera") != -1 &&
	   search(navigator.userAgent, "BeOS") != -1) {
	    // BeOS version of Opera
	    css = "opera.css";
	} else {
		// Netscape or Windows Opera
	    css = "netscape.css";
	}
    }

    document.writeln('<LINK REL="stylesheet" HREF="common/' + css + '">');
}
