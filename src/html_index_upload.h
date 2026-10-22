#pragma once

static constexpr const char *HTML_TARGZ_LINK = R"(<tr class='tableupload'>
     <td class='tableupload' align=left> -> <a href="./?action=targz">Download dir as tar.gz</a>
     </td>
</tr>)";

static constexpr const char *HTML_UPLOAD_FORM = R"(
    <form method='post' enctype='multipart/form-data'>
        <table align='center' width='80%' class='tableupload'>
            <tr width='100%' class='tableupload'>
                <td align=left class='tableupload'>
                    <label for='file'>Choose file to upload -> </label>
                    <input type='file' id='file' name='file' multiple>
                </td>
                <td align=right class='tableupload'>
                <input type='button' class='button' value='Upload' id='uploadbtn'>
                </td>
            </tr>
        </table>
    </form>
)";

// https://www.w3schools.com/js/js_cookies.asp
static constexpr const char *HTML_JS_UPLOAD = R"(
<script>
// From: https://www.w3schools.com/js/js_cookies.asp
function getCookie(cname) {
  let name = cname + '=';
  let decodedCookie = decodeURIComponent(document.cookie);
  let ca = decodedCookie.split(';');
  for(let i = 0; i <ca.length; i++) {
    let c = ca[i];
    while (c.charAt(0) == ' ') {
      c = c.substring(1);
    }
    if (c.indexOf(name) == 0) {
      return c.substring(name.length, c.length);
    }
  }
  return '';
}
document.addEventListener('DOMContentLoaded', function(){
    document.getElementById('uploadbtn').addEventListener('click', function(){
        var fd = new FormData();
        var files = document.getElementById('file').files;

        // Check file selected or not
        if(files.length == 0 ){
            alert('Please select a file.'); return;
        }
        for (var x = 0; x < files.length; x++) {
            fd.append('file', files[x]);
        }
        fetch('./.upload', {
            method: 'POST',
            headers: { 'X-CSRFToken': getCookie('CSRFToken') },
            body: fd
        })
        .then(function(response){
            if(!response.ok) { throw new Error('HTTP ' + response.status); }
            return response.text();
        })
        .then(function(response){
            if(response.trim() == 'true') { alert('File(s) uploaded'); location.reload(); }
            else{ alert('File(s) not uploaded'); }
        })
        .catch(function(){
            alert('File(s) not uploaded');
        });
    });
});
</script>
)";


