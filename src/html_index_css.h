#pragma once

// https://www.iconfinder.com/icons/314779/folder_icon
// https://www.iconfinder.com/icons/315179/document_text_icon
// https://www.iconfinder.com/icons/314895/document_text_icon

static constexpr const char *HTML_CSS_PAGE = R"(
<style>
        table,
        th,
        td {
            border: 1px solid rgb(171, 171, 172);
            border-collapse: collapse;
            border-spacing: 0;
            font:  Inconsolata, monospace, Courier New;
            padding: 4px;
        }
        .tableupload
        {
            border: 0px solid rgb(100, 100, 100);
            border-collapse: collapse;
            border-spacing: 0;
            font: Inconsolata, monospace, Courier New;
        }
        body {
            background-color: black;
            background-image: radial-gradient(rgba(0, 38, 87, 0.75), black 120%);
            height: 100vh;
            color: white;
            font: Inconsolata, monospace, Courier New;
            zoom: 80%;
        }
        a:link {
            color: rgb(255, 255, 255);
        }
        a:visited {
            color: rgb(194, 194, 194);
        }
        a:hover {
            color: rgb(199, 198, 255);
        }
        a:active {
            color: rgb(34, 148, 255);
        }
       .button{
        }
        .folder16 {
        background-image: url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAAAAXNSR0IArs4c6QAAAbJJREFUaEPtWTtOw0AQfROzoaDGfQ5ATQFllJwBTgISShZHQrkCB4ADQAMHgALuQAvKR3Far/CgREqUKIbYml0jR+vSnnkz771ZrdcmVPyiivcPT+C/HfQOeAeECqyNUNgxbTCOmBBIcE1t7zbWFEsw8uYuCYRXyT3X6Cxv4l9xBFwPIqVtYG3DmBOYKc/A07bg3M+ZYxOoRhkuzAkcdswlgJvcDeYILMuFhQOagW6OvgqE8DRJVGPap0mBpMKhDglgts07XwtOCYA5BlG/sKwZCcw8wbd6HN3Q5+pjtwRsdL6KwRynAdpjXX9b3K4WAQAp8DqO1EllCRBzMujV9ytLYNb4MFLLDbhyI5RNQJsmp2jaXnOu8IaRulgbIVeFysDdkRHqGAevEu7038FF7B1wNy5ZyH6EytV7s5p3wDsgVMCPkFBAcbp3QCyhEMA7IBRQnL7hgItPi+IufwHIPtRr0+IUz66K2sRl4GUUqdONI2XYTe6Y6dxmMftYPEkZ7XGv/p55Jg61aaUpjgk4sF9chsjMHxSoh6Gmr1Uk/5NPpqs82zsg11CG4B2Q6SfPrrwDP1yR2jE/Y55DAAAAAElFTkSuQmCC");
        background-repeat: no-repeat;
        background-size: 16px 16px;
        padding-left: 24px; padding-top: 2px; padding-bottom: 2px;
        }
        .file16 {
        background-image: url("data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAADAAAAAwCAYAAABXAvmHAAABhGlDQ1BJQ0MgUFJPRklMRQAAKJF9kT1Iw0AcxV9TRdGKoh2KOASpThZERRy1CkWoEGqFVh1MLv2CJg1Jiouj4Fpw8GOx6uDirKuDqyAIfoC4uTkpukiJ/0sKLWI9OO7Hu3uPu3eAUC0yzWobBzTdNhOxqJhKr4odr+hGH/oRwrDMLGNOkuJoOb7u4ePrXYRntT735+hRMxYDfCLxLDNMm3iDeHrTNjjvEwdZXlaJz4nHTLog8SPXFY/fOOdcVnhm0Ewm5omDxGKuiZUmZnlTI54iDquaTvlCymOV8xZnrVhm9XvyFwYy+soy12kOIYZFLEGCCAVlFFCEjQitOikWErQfbeEfdP0SuRRyFcDIsYASNMiuH/wPfndrZScnvKRAFGh/cZyPEaBjF6hVHOf72HFqJ4D/GbjSG/5SFZj5JL3S0MJHQO82cHHd0JQ94HIHCD0Zsim7kp+mkM0C72f0TWlg4BboWvN6q+/j9AFIUlfxG+DgEBjNUfZ6i3d3Nvf275l6fz9uIXKlTWKFwgAAAAZiS0dEAP8A/wD/oL2nkwAAAbdJREFUaN7tmK1uFVEUhb9N+RHFoRAIkqIaqiAhgeDAAApXDekTEKrIoKAIIOEBMDdUNzwA5hIUoBAFR3JxVFVQEB9mMJObdC5zzrmQe5Y5yWSfn7X3rL3ODFRULDZi1gnqdWANWBq49ygiJkXZqiPT4YN6suThr7Ubb6rHBqzTtOv8UF+rgyp5ZIbYC+34NCJ+JcjJHeAG8GzIIkdniD0BEBE/k4gvYqSeAx6oXyLiRW4COdAAZ4Hn6teI2ClKQL3fM/RtRIynVEH1bktipF6NiI+5RNyodp71RXPIOqfUz+pEPVOsAhERifTwXb0FvAN22krsp+5CqXDQZv14h8QucBtYBbb7ttd5EHjfjvemkHgDbAA3h7bXbBpo573qM6lEG93qGTfuZHpdfQlcApanxF8BLmevQMZrS++95qGBpChlZH9tdrk18DghgYddrfw3RlbqOv1PYrEJmBbNPL4HthImc1ycQERsVg1UI6tGVjVQCVQjq0ZWjawaWTWy6gMLQeBbK9yVAuda+bPfoa/xDB3nNPAJ2AO2gf1Mhz8PrANPkvuMelEdqwfmw0R91P1zXVGRCb8BsbyZP/dGyOsAAAAASUVORK5CYII=");
        background-repeat: no-repeat;
        background-size: 16px 16px;
        padding-left: 24px; padding-top: 2px; padding-bottom: 2px;
        }
</style>
)";


