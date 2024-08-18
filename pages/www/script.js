//recuperer un cookie et l'afficher

function lireCookie(nom) {
    var nomEQ = nom + "=";
    var ca = document.cookie.split(';');
    for(var i=0;i < ca.length;i++) {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1,c.length);
        if (c.indexOf(nomEQ) == 0) return c.substring(nomEQ.length,c.length);
    }
    return null;
}

//recuperer le cookie
var cookie = lireCookie("WebServCookie");

console.log(cookie);
//afficher le cookie
document.write("<div style=\"color:green\"; \"font-weight: bold\">" + "Session Cookie: " + cookie + "</div>");
