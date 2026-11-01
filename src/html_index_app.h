#pragma once

#include <string>
#include <boost/algorithm/string/replace.hpp>
#include <Mantids30/Helpers/encoders.h>

// Build the HTML page with a custom title.
// If appTitle is empty, defaults to "Unmanarc's Media Browser".
inline std::string buildHTMLApp(const std::string &appTitle)
{
    const std::string title = appTitle.empty() ? "Unmanarc's Media Browser" : appTitle;
    const std::string escaped = Mantids30::Helpers::Encoders::htmlEscape(title);

    static const std::string rawHTML = R"uSWAPP(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>__APP_TITLE__</title>
<style>
:root{
  --bg:#0e1116; --panel:#151a21; --panel2:#1c232c; --border:#27303b;
  --fg:#e0e6ed; --fg-dim:#8b98a8; --accent:#3b82f6; --accent2:#60a5fa;
  --danger:#ef5350; --gold:#f6c945;
  --item-h:56px;
  --thumb-h:72px;
}
*{box-sizing:border-box;}
html,body{margin:0;height:100%;}
body{
  background:var(--bg); color:var(--fg);
  font:14px/1.4 "Segoe UI", system-ui, -apple-system, Roboto, Helvetica, Arial, sans-serif;
  overflow:hidden;
}
#app{display:grid;height:100vh;grid-template-rows:52px 48px 1fr 40px;}

/* ----- A: toolbar ----- */
#zoneA{
  display:flex; align-items:center; gap:6px; padding:0 10px;
  background:var(--panel); border-bottom:1px solid var(--border);
  flex-wrap:nowrap; overflow-x:auto;
}
#zoneA .brand{font-weight:600; margin-right:8px; color:var(--accent2); white-space:nowrap;}

/* ----- C: location bar ----- */
#zoneC{
  display:flex; align-items:center; gap:6px; padding:0 10px;
  background:var(--panel2); border-bottom:1px solid var(--border);
}
#pathInput{
  flex:1; min-width:0; background:var(--bg); color:var(--fg);
  border:1px solid var(--border); border-radius:6px; padding:6px 10px; font:inherit;
}

/* ----- F: status bar (becomes the top header in the phone view) ----- */
#zoneF{
  display:flex; justify-content:flex-end; align-items:center; gap:8px;
  padding:0 10px; background:var(--panel); border-top:1px solid var(--border);
}
/* Up/Refresh/Back buttons only exist in the phone header; hidden on desktop. */
#zoneF .mnav{display:none;}
/* The directory-name row is a phone-header element; hidden on desktop.
   (.mbar is the wrapper of the icon bar, so it must stay visible here.) */
#zoneF .mtitle-row{display:none;}

/* ----- B + D + E ----- */
#zoneMid{display:grid; grid-template-columns:260px 1fr 300px; min-height:0;}
#zoneE{
  display:flex; flex-direction:column; min-height:0;
  background:var(--panel); border-left:1px solid var(--border);
  overflow:auto; padding:16px;
}
#detailContent{flex:1 1 auto;}
#selActions{
  flex:none; display:none; justify-content:center; gap:12px;
  padding-top:16px; border-top:1px solid var(--border); margin-top:8px;
}
#selActions.show{display:flex;}
#selActions .btn{min-width:110px; justify-content:center;}
#detailContent h3{margin:0 0 12px; font-size:14px; word-break:break-all;}
#detailContent .d-thumb{display:flex; justify-content:center; margin-bottom:12px;}
#detailContent .d-thumb img{max-width:100%; max-height:260px; border-radius:8px; object-fit:contain; cursor:pointer;}
#detailContent table{width:100%; border-collapse:collapse; font-size:12.5px;}
#detailContent td{padding:5px 0; vertical-align:top;}
#detailContent td.k{color:var(--fg-dim); padding-right:12px; white-space:nowrap; width:38%;}
#detailContent td.v{word-break:break-word;}
#detailContent .exif h2{margin:16px 0 8px; font-size:12px; text-transform:uppercase; letter-spacing:.06em; color:var(--fg-dim);}
#detailContent h2.dirinfo{margin:16px 0 8px; font-size:12px; text-transform:uppercase; letter-spacing:.06em; color:var(--fg-dim);}
#detailContent .empty{color:var(--fg-dim); font-size:13px; text-align:center; padding:40px 0;}
#zoneB{
  background:var(--panel); border-right:1px solid var(--border);
  overflow:auto; padding:6px 0;
}
#zoneD{
  position:relative; overflow:auto; padding:12px;
}
#rubberBand{
  position:absolute; z-index:50; display:none;
  border:1px solid var(--accent); background:rgba(59,130,246,.15);
  border-radius:2px; pointer-events:none;
}

/* Toolbar / icon buttons */
.btn{
  display:inline-flex; align-items:center; gap:6px;
  background:var(--panel2); color:var(--fg); border:1px solid var(--border);
  border-radius:6px; padding:6px 10px; cursor:pointer; font:inherit; white-space:nowrap;
}
.btn:hover{background:#232c38; border-color:#3a4757;}
.btn:disabled{opacity:.4; cursor:not-allowed;}
.btn.icon{width:34px; height:34px; padding:0; justify-content:center;}
.btn.icon>i{width:18px; height:18px; background-repeat:no-repeat; background-position:center; background-size:contain;}
.btn.primary{background:var(--accent); border-color:var(--accent); color:#fff;}
.btn.danger{background:var(--danger); border-color:var(--danger); color:#fff;}
.sel{
  background:var(--panel2); color:var(--fg); border:1px solid var(--border);
  border-radius:6px; padding:5px 8px; font:inherit; cursor:pointer;
}

/* icons (data:image svg) */
i.ic{display:inline-block; width:18px; height:18px; background-repeat:no-repeat; background-position:center; background-size:contain;}
i.ic-back{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M20 11H7.83l5.59-5.59L12 4l-8 8 8 8 1.41-1.41L7.83 13H20v-2z'/%3E%3C/svg%3E");}
i.ic-up{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M12 8l-6 6 1.41 1.41L12 10.83l4.59 4.58L18 14z'/%3E%3C/svg%3E");}
i.ic-forward{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M12 4l-1.41 1.41L16.17 11H4v2h12.17l-5.58 5.59L12 20l8-8z'/%3E%3C/svg%3E");}
i.ic-refresh{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M17.65 6.35A7.95 7.95 0 0 0 12 4a8 8 0 1 0 8 8h-2a6 6 0 1 1-1.76-4.24L13 11h7V4z'/%3E%3C/svg%3E");}
i.ic-fullscreen{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M7 14H5v5h5v-2H7v-3zm-2-4h2V7h3V5H5v5zm12 7h-3v2h5v-5h-2v3zM14 5v2h3v3h2V5h-5z'/%3E%3C/svg%3E");}
i.ic-newfolder{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23f6c945' d='M10 4H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-8l-2-2z'/%3E%3Cpath fill='%23e0e6ed' d='M11 12h2v2h2v2h-2v2h-2v-2H9v-2h2z'/%3E%3C/svg%3E");}
i.ic-upload{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M9 16h6v-6h4l-7-7-7 7h4v6zm-4 2h14v2H5v-2z'/%3E%3C/svg%3E");}
i.ic-download{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M19 9h-4V3H9v6H5l7 7 7-7zM5 18v2h14v-2H5z'/%3E%3C/svg%3E");}
i.ic-delete{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23ef5350' d='M6 7h12l-1 14H7L6 7zm4-5h4l1 2H9l1-2zM9 11v7h2v-7H9zm4 0v7h2v-7h-2z'/%3E%3C/svg%3E");}
i.ic-list{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M4 5h2v2H4zm0 4h2v2H4zm0 4h2v2H4zm0 4h2v2H4zM8 5h12v2H8zm0 4h12v2H8zm0 4h12v2H8zm0 4h12v2H8z'/%3E%3C/svg%3E");}
i.ic-grid{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M4 4h7v7H4zm9 0h7v7h-7zM4 13h7v7H4zm9 0h7v7h-7z'/%3E%3C/svg%3E");}
i.ic-close{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23e0e6ed' d='M18.3 5.71 12 12l6.3 6.29-1.41 1.41L12 13.41l-4.89 4.89-1.41-1.41L10.59 12 5.7 6.71 7.11 5.3 12 10.59l4.89-4.89z'/%3E%3C/svg%3E");}

/* File type icons */
i.ic-folder{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%23f6c945' d='M10 4H4a2 2 0 0 0-2 2v12a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V8a2 2 0 0 0-2-2h-8l-2-2z'/%3E%3C/svg%3E");}
i.ic-file{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%2390a4b7' d='M6 2a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8l-6-6H6zm7 1.5L18 8.5H13V3.5z'/%3E%3C/svg%3E");}
i.ic-image{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%237fb2ff' d='M21 5v14a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2zM8.5 9a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM7 14l2-2 2 2 3-4 3 5-4 5-6-6z'/%3E%3C/svg%3E");}
i.ic-album{background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24'%3E%3Cpath fill='%234caf50' d='M4 5a2 2 0 0 1 2-2h12a2 2 0 0 1 2 2v14a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2V5zm3 1.5a1.5 1.5 0 1 0 0 3 1.5 1.5 0 0 0 0-3zM5.5 17.5l3.5-4.5 3 4 2.5-3 4 3.5v1.5h-13v-1.5z'/%3E%3C/svg%3E");}

/* ----- tree (B) ----- */
#zoneB h3{margin:2px 12px; font-size:12px; text-transform:uppercase; letter-spacing:.06em; color:var(--fg-dim);}
.tree-row{
  display:flex; align-items:center; gap:4px; height:28px; cursor:pointer;
  color:var(--fg); border-radius:4px; margin:0 4px; padding-right:4px; user-select:none;
}
.tree-row:hover{background:#202933;}
.tree-row.active{background:#1e3a5f;}
.tree-row .tw{width:16px; text-align:center; color:var(--fg-dim); flex:none;}
.tree-row .lbl{white-space:nowrap; overflow:hidden; text-overflow:ellipsis;}
.tree-row i.ic-folder{width:16px; height:16px; flex:none;}

/* ----- D files ----- */
#filesGrid{display:grid; gap:10px;}
#filesGrid.view-grid{display:flex; flex-wrap:wrap; gap:12px; align-content:flex-start; padding-bottom:24px;}
#filesGrid.view-list{grid-template-columns:1fr;}
.item{
  border:1px solid transparent; border-radius:8px; cursor:pointer;
  display:flex; flex-direction:column; align-items:center; text-align:center;
  padding:10px 8px; gap:6px; background:transparent; color:var(--fg);
}
#filesGrid.view-grid .item{position:relative; width:max-content; max-width:none; height:auto;}
#filesGrid.view-list .item{
  flex-direction:row; text-align:left; padding:6px 10px;
  min-height:var(--item-h); height:auto;
  gap:10px; border-bottom:1px solid var(--border); border-radius:0;
}
.item:hover{background:#161d26;}
.item.selected{background:#1e3a5f; border-color:var(--accent);}
.item.active{border-color:var(--accent2); background:#182433;}
.item.selected.active{background:#1e3a5f; border-color:var(--accent2);}
.item .thumb{display:flex; align-items:center; justify-content:center; border-radius:6px; flex:none;}
.item .thumb img{display:block;}
/* list view thumbnails = square sized by the selected icon size */
#filesGrid.view-list .item .thumb{width:var(--thumb-h); height:var(--thumb-h); overflow:hidden;}
#filesGrid.view-list .item .thumb img{width:100%; height:100%; object-fit:cover;}
#filesGrid.view-list .item .thumb i.ic{width:calc(var(--thumb-h) - 8px); height:calc(var(--thumb-h) - 8px);}
/* icon view thumbnails = fixed height, natural aspect-ratio width */
#filesGrid.view-grid .item .thumb{height:var(--thumb-h); min-width:var(--thumb-h);}
#filesGrid.view-grid .item .thumb img{height:var(--thumb-h); width:auto; max-width:none;}
#filesGrid.view-grid .item .thumb i.ic{width:calc(var(--thumb-h) - 12px); height:calc(var(--thumb-h) - 12px);}
.item .name{
  width:100%; overflow:hidden; text-overflow:ellipsis;
  display:-webkit-box; -webkit-line-clamp:2; -webkit-box-orient:vertical;
  word-break:break-all; font-size:12.5px;
}
#filesGrid.view-list .item .name{font-size:inherit;-webkit-line-clamp:1;}
/* Grid view: one line (the text is already truncated by shortName) so a name
   can never stretch the card past the measured page-wide width. */
#filesGrid.view-grid .item .name{-webkit-line-clamp:1; word-break:normal; overflow-wrap:normal;}
.item .meta{font-size:11px; color:var(--fg-dim); white-space:nowrap;}
#filesGrid.view-list .item .meta{margin-left:auto; display:flex; gap:16px;}
#filesGrid.view-list .item .meta span{min-width:70px;}
#emptyMsg{padding:40px; text-align:center; color:var(--fg-dim);}

/* ----- mobile (phone) view -----
   The status bar (F) becomes a fixed top header with Up / Refresh / Back
   buttons; the file view (D) fills the rest. Header (A), location bar (C),
   tree (B) and details (E) are hidden. The ".." entry is no longer rendered
   in the list: navigation is done through the header buttons. ----- */
@media (max-width: 640px), (max-height: 520px) {
  #app{grid-template-rows:66px 1fr;}
  #zoneA, #zoneC, #zoneB, #zoneE{display:none !important;}
  #zoneMid{display:block; min-height:0;}
  /* Keep #zoneD inside the grid track's 1fr height (it has overflow:auto)
     so its vertical scrollbar stays visible on phones. */
  #zoneD{padding:8px; height:100%; min-height:0;}
  /* F moves to the top row and becomes a two-row header:
     row 1 = the icon buttons (Up/Refresh/Back, view, size, sort),
     row 2 = the current directory name, centered and emphasized.
     (No extra grid children are added: #zoneF itself wraps the two rows
     so #app's explicit 2-row grid stays exactly aligned.) */
  #zoneF{
    order:-1; flex:0 0 66px; height:66px;
    display:flex; flex-direction:column;
    border-top:none; border-bottom:1px solid var(--border);
    gap:0; padding:0; overflow:hidden;
  }
  /* Row 1: the icon bar. It is sized down from the desktop bar (smaller
     buttons, selects and gaps) so that on phones the items shrink to fit the
     viewport width instead of overflowing and hiding icons. JS measures the
     natural (unclamped) width of the row and, when it overflows, scales it
     with a transform so every control stays visible at any phone size. */
  #zoneF .mbar{
    display:flex; align-items:center; flex:0 0 34px; height:34px;
    gap:4px; padding:0 6px; overflow-x:visible; overflow-y:hidden;
  }
  #zoneF .mbar .btn.icon{width:28px; height:28px;}
  #zoneF .mbar .btn.icon>i{width:16px; height:16px;}
  #zoneF .mnav{display:inline-flex;}
  #zoneF .mnav-group{display:inline-flex; align-items:center; gap:4px; flex:none;}
  #zoneF .spacer{flex:1 1 auto; min-width:4px;}
  #zoneF .sel{max-width:88px; flex-shrink:1; min-width:0; padding:4px 6px;}
  /* Row 2: the current directory name, centered and emphasized. */
  #zoneF .mtitle-row{
    display:flex; align-items:center; flex:1 1 auto;
    min-height:0; padding:0 12px; border-top:1px solid var(--border);
  }
  #zoneF .mtitle{
    width:100%; margin:0;
    font-weight:700; font-size:15px; color:var(--fg);
    text-align:center; white-space:nowrap; overflow:hidden;
    text-overflow:ellipsis;
  }
  .item{padding:12px 8px;}
  .item .name{font-size:13px; -webkit-line-clamp:3;}
  #filesGrid.view-grid .item .thumb i.ic{width:calc(var(--thumb-h) - 6px); height:calc(var(--thumb-h) - 6px);}
}

/* context menu */
#ctxMenu{
  position:fixed; z-index:1000; min-width:200px; background:var(--panel2);
  border:1px solid var(--border); border-radius:8px; padding:4px; box-shadow:0 8px 30px rgba(0,0,0,.5);
  display:none;
}
#ctxMenu .mi{
  display:flex; align-items:center; gap:10px; padding:8px 10px; border-radius:5px;
  cursor:pointer; font-size:13px;
}
#ctxMenu .mi:hover{background:#232c38;}
#ctxMenu .mi.disabled{opacity:.4; pointer-events:none;}
#ctxMenu .sep{height:1px; background:var(--border); margin:4px 0;}

/* modal */
#overlay{position:fixed; inset:0; background:rgba(0,0,0,.5); display:none; align-items:center; justify-content:center; z-index:2000;}
#overlay.open{display:flex;}
.dialog{background:var(--panel2); border:1px solid var(--border); border-radius:10px; padding:20px; min-width:320px; box-shadow:0 12px 40px rgba(0,0,0,.6);}
.dialog h3{margin:0 0 14px;}
.dialog input[type=text]{width:100%; background:var(--bg); color:var(--fg); border:1px solid var(--border); border-radius:6px; padding:8px 10px; font:inherit; margin-bottom:12px;}
.dialog .actions{display:flex; justify-content:flex-end; gap:8px;}

/* album (full-screen media layer) */
#albumLayer{
  position:fixed; inset:0; z-index:2500; background:#000;
  display:none; align-items:center; justify-content:center;
}
#albumLayer.open{display:flex;}
#albumMedia{
  display:flex; align-items:center; justify-content:center;
  width:100%; height:100%; padding:64px 16px 16px;
}
#albumImg, #albumVideo{max-width:100%; max-height:100%; object-fit:contain; display:none;}
#albumImg.show, #albumVideo.show{display:block;}
/* real click targets: half of the screen each; the visible buttons are only decorative */
.album-hitzone{position:absolute; top:0; bottom:0; width:50%; z-index:10;}
.album-hitzone.left{left:0;}
.album-hitzone.right{right:0;}
.album-nav{
  position:absolute; top:50%; transform:translateY(-50%);
  z-index:20; pointer-events:none; border:0; background:transparent; cursor:pointer;
  font-size:64px; line-height:1; color:rgba(255,255,255,.35); font-family:Georgia, serif;
  padding:0 22px; user-select:none;
}
.album-hitzone:hover + .album-nav, .album-nav:hover{color:rgba(255,255,255,.85);}
.album-nav.left{left:0;}
.album-nav.right{right:0;}
.album-close{
  position:absolute; top:10px; right:10px; z-index:31;
  background:rgba(255,255,255,.08); border-color:transparent; color:#fff;
}
.album-close:hover{background:rgba(255,255,255,.2);}
#albumFooter{
  position:absolute; left:0; right:0; top:0; z-index:30;
  display:flex; align-items:center; gap:12px; padding:10px 56px 10px 16px;
  background:rgba(20,24,30,.82); backdrop-filter:blur(6px);
  font-size:13px; color:#cfd8e3;
}
#albumName{font-weight:600; color:#fff; word-break:break-all; max-width:28%;}
#albumPos{color:#8b98a8; white-space:nowrap;}
#albumExif{margin-left:auto; text-align:right; color:#8b98a8; font-size:12px; word-break:break-word; max-width:50%;}
#albumExif .chip{display:inline-block; margin-left:10px; white-space:nowrap;}
#albumExif .chip b{color:#cfd8e3; font-weight:600;}

/* toast */
#toast{position:fixed; bottom:18px; left:50%; transform:translateX(-50%); background:var(--panel2); border:1px solid var(--border); border-radius:8px; padding:10px 16px; z-index:3000; display:none; box-shadow:0 6px 20px rgba(0,0,0,.5);}

/* drag & drop upload overlay */
#dropOverlay{
  position:fixed; inset:0; z-index:2900; display:none;
  align-items:center; justify-content:center;
  background:rgba(0,0,0,.55);
}
#dropOverlay.open{display:flex;}
#dropOverlay .dropBox{
  border:3px dashed var(--accent); border-radius:16px;
  background:rgba(21,26,33,.85);
  padding:64px 80px; text-align:center;
  box-shadow:0 0 40px rgba(59,130,246,.35) inset;
}
#dropOverlay .dropBox .ic{width:56px; height:56px; margin:0 auto 16px; display:block;}
#dropOverlay .dropBox .t1{font-size:18px; font-weight:600; margin-bottom:6px;}
#dropOverlay .dropBox .t2{color:var(--fg-dim); font-size:13px;}
</style>
</head>
<body>
<div id="app">
  <!-- A -->
  <header id="zoneA">
    <span class="brand">__APP_TITLE__</span>
    <span style="flex:1"></span>
    <button class="btn icon" id="btnFullscreen" title="Fullscreen"><i class="ic ic-fullscreen"></i></button>
  </header>

  <!-- C -->
  <div id="zoneC">
    <button class="btn icon" id="btnBack" title="Back"><i class="ic ic-back"></i></button>
    <button class="btn icon" id="btnUp" title="Up"><i class="ic ic-up"></i></button>
    <button class="btn icon" id="btnRefresh" title="Refresh"><i class="ic ic-refresh"></i></button>
    <span class="sep" style="width:1px;height:24px;background:var(--border);margin:0 4px;"></span>
    <button class="btn icon" id="btnNewFolder" title="New folder"><i class="ic ic-newfolder"></i></button>
    <button class="btn icon" id="btnUpload" title="Upload files"><i class="ic ic-upload"></i></button>
    <input id="pathInput" type="text" placeholder="Location" spellcheck="false">
    <button class="btn icon" id="btnGo" title="Go"><i class="ic ic-forward"></i></button>
    <input id="filterInput" type="text" placeholder="Filter..." style="max-width:180px;">
  </div>

  <!-- B + D -->
  <div id="zoneMid">
    <aside id="zoneB">
      <h3>Tree</h3>
      <div id="tree"></div>
    </aside>
    <main id="zoneD" oncontextmenu="return false;">
      <div id="filesGrid" class="view-grid"></div>
      <div id="rubberBand"></div>
      <div id="emptyMsg" style="display:none;">Empty directory</div>
    </main>
    <aside id="zoneE">
      <div id="detailContent"></div>
      <div id="selActions">
        <button class="btn" id="btnDownload" title="Download selected">Download</button>
        <button class="btn danger" id="btnDelete" title="Delete selected">Delete</button>
      </div>
    </aside>
  </div>

  <!-- F: status bar (becomes the top header in the phone view; in the phone
       view it wraps two rows: the icon bar + the directory name) -->
  <footer id="zoneF">
    <div class="mbar">
      <span class="mnav-group">
        <button class="btn icon mnav" id="btnMUp" title="Up"><i class="ic ic-up"></i></button>
        <button class="btn icon mnav" id="btnMRefresh" title="Refresh"><i class="ic ic-refresh"></i></button>
        <button class="btn icon mnav" id="btnMBack" title="Back"><i class="ic ic-back"></i></button>
      </span>
      <span class="spacer"></span>
      <button class="btn icon" id="btnGrid" title="Grid view"><i class="ic ic-grid"></i></button>
      <button class="btn icon" id="btnList" title="List view"><i class="ic ic-list"></i></button>
      <button class="btn icon" id="btnAlbum" title="Album view"><i class="ic ic-album"></i></button>
      <select id="sizeSelect" class="sel" title="Icon size">
        <option value="48">48 px</option>
        <option value="72" selected>72 px</option>
        <option value="96">96 px</option>
        <option value="128">128 px</option>
        <option value="192">192 px</option>
        <option value="256">256 px</option>
        <option value="384">384 px</option>
        <option value="512">512 px</option>
      </select>
      <select id="sortSelect" class="sel" title="Sort">
        <option value="name:asc" selected>Name</option>
        <option value="name:desc">Name &#8595;</option>
        <option value="date:desc">Date</option>
        <option value="date:asc">Date &#8593;</option>
        <option value="size:desc">Size</option>
        <option value="size:asc">Size &#8593;</option>
        <option value="type:asc">Type</option>
      </select>
    </div>
    <div class="mtitle-row">
      <span class="mtitle" id="mTitle"></span>
    </div>
  </footer>
</div>

<div id="ctxMenu"></div>
<div id="overlay"><div class="dialog" id="dialog"></div></div>

<!-- Album (full-screen media) layer -->
<div id="albumLayer">
  <a id="albumLinkPrev" class="album-hitzone left" href="#" title="Previous"></a>
  <a id="albumLinkNext" class="album-hitzone right" href="#" title="Next"></a>
  <button id="albumBtnPrev" class="album-nav left" title="Previous">&#8249;</button>
  <button id="albumBtnNext" class="album-nav right" title="Next">&#8250;</button>
  <button class="btn icon album-close" id="btnAlbumClose" title="Close album"><i class="ic ic-close"></i></button>
  <div id="albumMedia">
    <img id="albumImg" alt="">
    <video id="albumVideo" controls playsinline preload="metadata"></video>
  </div>
  <header id="albumFooter">
    <span id="albumName"></span>
    <span id="albumPos"></span>
    <span id="albumExif"></span>
  </header>
</div>

<div id="toast"></div>
<div id="dropOverlay">
  <div class="dropBox">
    <i class="ic ic-upload"></i>
    <div class="t1">Drop files to upload</div>
    <div class="t2">They will be saved in the current directory</div>
  </div>
</div>
<input type="file" id="fileInput" multiple style="position:absolute;left:-9999px;top:0;width:1px;height:1px;opacity:0;">
<script>
'use strict';
var $ = function(s, r){ return (r||document).querySelector(s); };
var $$ = function(s, r){ return Array.prototype.slice.call((r||document).querySelectorAll(s)); };

// Phone view (CSS media query hides the header/location/tree/details panels
// and turns the footer into a top header with Up/Refresh/Back buttons).
var mqlMobile = window.matchMedia('(max-width: 640px), (max-height: 520px)');
function isMobile(){ return mqlMobile.matches; }
// Re-render when the viewport crosses the phone breakpoint so the panels
// show/hide and the header buttons appear/disappear without a manual navigation.
function onMobileBreakpointChange(){ renderFiles(); renderToolbar(); }
if (mqlMobile.addEventListener) mqlMobile.addEventListener('change', onMobileBreakpointChange);
else if (mqlMobile.addListener) mqlMobile.addListener(onMobileBreakpointChange);

// Navigation history for the mobile "Back" button (independent of the browser
// history, which is driven by the URL hash).
var navHistory = [];   // normalised paths, oldest -> newest
var navIndex = -1;     // index into navHistory of the current path
var navSuppressed = false; // true while a back/forward step is in flight

function getCookie(name){
  var m = document.cookie.match(new RegExp('(?:^|; )' + name + '=([^;]*)'));
  return m ? decodeURIComponent(m[1]) : '';
}
function enc(s){ return encodeURIComponent(s); }

async function api(path, opts){
  opts = opts || {};
  opts.credentials = 'same-origin';
  if (opts.method && opts.method !== 'GET'){
    opts.headers = opts.headers || {};
    opts.headers['X-CSRFToken'] = getCookie('CSRFToken');
  }
  var res = await fetch(path, opts);
  try { return await res.json(); } catch(e){ return {ok:false, error:'Bad response'}; }
}

function toast(msg){
  var t = $('#toast');
  t.textContent = msg;
  t.style.display = 'block';
  clearTimeout(t._h);
  t._h = setTimeout(function(){ t.style.display = 'none'; }, 2600);
}

var state = {
  path: '/',
  flags: { canUpload:false, canOverwrite:false, canArchive:false, canExecute:false, version:'' },
  upload_here: true,
  view: 'grid',
  filter: '',
  sortKey: 'name',
  sortDir: 'asc',
  thumbSize: 72,
  selected: [],   // array of names
  items: [],
  activeIndex: -1  // index into visibleNames() for keyboard cursor
};
var treeCache = {};   // path -> nodes[]
var expanded = {};    // path -> true
var thumbCache = {};   // "path@size" -> data: URI
var thumbPending = {}; // "path@size" -> true while a thumb fetch is in-flight
var thumbQueue = [];   // pending "path@size" keys shared by the workers
var thumbQueued = {};  // "path@size" -> true while queued
var thumbWorkersRunning = 0;
var THUMB_CONCURRENCY = 4;  // max simultaneous thumb fetches (one per worker)

// ---------------- ALBUM (full-screen media) ----------------
var albumIdx = -1;      // index into albumList() while the album is open
var albumListCache = [];
var albumExifCache = {}; // full path -> chip html (already rendered)

function isVideo(name){
  return /\.(mp4|webm|ogv|ogg|mov|m4v)$/i.test(name);
}
function isMedia(it){
  return it.type === 'file' && (isImage(it.name) || isVideo(it.name));
}
// Images + videos only (mixed folders: nothing else), respecting filter + sort.
function albumList(){
  if (!albumListCache.length){
    albumListCache = sortItems(state.items.filter(isMedia));
  }
  return albumListCache;
}
function albumOpen(){ return $('#albumLayer').classList.contains('open'); }

function openAlbum(preferName){
  var list = albumList();
  if (!list.length){ toast('No images or videos here'); return; }
  albumIdx = 0;
  if (preferName){
    var i = list.findIndex(function(it){ return it.name === preferName; });
    if (i >= 0) albumIdx = i;
  } else if (state.selected.length === 1){
    var j = list.findIndex(function(it){ return it.name === state.selected[0]; });
    if (j >= 0) albumIdx = j;
  }
  $('#albumLayer').classList.add('open');
  renderAlbum();
}

function closeAlbum(){
  var v = $('#albumVideo');
  v.pause();
  v.removeAttribute('src');
  v.load();
  $('#albumLayer').classList.remove('open');
  albumIdx = -1;
}

function albumNav(dir){
  var list = albumList();
  if (!list.length) return;
  albumIdx = (albumIdx + dir + list.length) % list.length;
  renderAlbum();
}

function mediaUrl(name){ return '?act=download&file=' + enc(itemPath(name)); }

function renderAlbum(){
  var list = albumList();
  if (!list.length){ closeAlbum(); return; }
  albumIdx = Math.max(0, Math.min(albumIdx, list.length - 1));
  var it = list[albumIdx];
  var img = $('#albumImg'), vid = $('#albumVideo');

  // Stop the previous media.
  if (vid.classList.contains('show')){ vid.pause(); }
  vid.removeAttribute('src'); vid.load();
  img.classList.remove('show');
  vid.classList.remove('show');

  $('#albumName').textContent = it.name;
  $('#albumPos').textContent = (albumIdx + 1) + ' / ' + list.length;
  $('#albumExif').innerHTML = '';

  if (isVideo(it.name)){
    vid.src = mediaUrl(it.name);
    vid.classList.add('show');
    vid.play().catch(function(){});
    vid.onloadedmetadata = function(){
      var dur = isFinite(vid.duration) ? Math.round(vid.duration) : 0;
      if (dur)
        $('#albumPos').textContent = (albumIdx + 1) + ' / ' + list.length +
          '   \u00b7   ' + Math.floor(dur/60) + ':' + ('0' + dur%60).slice(-2);
    };
  } else {
    img.src = mediaUrl(it.name);
    img.classList.add('show');
    loadAlbumExif(it);
  }
  preloadAlbumNext();
}

// Warm the next image so advancing feels instant.
function preloadAlbumNext(){
  var list = albumList();
  var nxt = list[(albumIdx + 1) % list.length];
  if (nxt && !isVideo(nxt.name)){
    var p = new Image();
    p.src = mediaUrl(nxt.name);
  }
}

async function loadAlbumExif(it){
  var full = itemPath(it.name);
  if (albumExifCache[full] !== undefined){
    $('#albumExif').innerHTML = albumExifCache[full];
    return;
  }
  var html = '';
  try {
    var r = await api('?act=detail&file=' + enc(full));
    if (r && r.ok && r.data){
      var d = r.data, chips = [];
      if (d.width && d.height)
        chips.push('<span class="chip"><b>' + d.width + '\u00d7' + d.height + '</b></span>');
      var e = d.exif || {};
      if (e.Make || e.Model)
        chips.push('<span class="chip"><b>' + escapeHtml([e.Make, e.Model].filter(Boolean).join(' ')) + '</b></span>');
      if (e.LensModel)
        chips.push('<span class="chip">' + escapeHtml(String(e.LensModel)) + '</span>');
      var dt = e.DateTimeOriginal || e.CreateDate;
      if (dt){
        var m = String(dt).match(/^(\d{4}:\d{2}:\d{2})[ T](\d{2}:\d{2}(?::\d{2})?)?/);
        var ds = m ? m[1] : String(dt);
        var ts = m && m[2] ? ' ' + m[2] : '';
        chips.push('<span class="chip">' + escapeHtml(ds + ts) + '</span>');
      }
      if (e.ExposureTime && e.FNumber)
        chips.push('<span class="chip">' + escapeHtml(String(e.ExposureTime)) + 's \u00b7 f/' + escapeHtml(String(e.FNumber)) + '</span>');
      else if (e.ExposureTime)
        chips.push('<span class="chip">' + escapeHtml(String(e.ExposureTime)) + 's</span>');
      else if (e.FNumber)
        chips.push('<span class="chip">f/' + escapeHtml(String(e.FNumber)) + '</span>');
      var iso = e.ISO || e.ISOSpeedRatings;
      if (iso) chips.push('<span class="chip">ISO ' + escapeHtml(String(iso).split('/')[0]) + '</span>');
      if (e.FocalLength)
        chips.push('<span class="chip">' + escapeHtml(String(e.FocalLength).split('/')[0]) + 'mm</span>');
      if (e.GPSLatitude && e.GPSLongitude){
        var lat = parseFloat(e.GPSLatitude) * (String(e.GPSLatitudeRef || '').indexOf('S') >= 0 ? -1 : 1);
        var lon = parseFloat(e.GPSLongitude) * (String(e.GPSLongitudeRef || '').indexOf('W') >= 0 ? -1 : 1);
        chips.push('<span class="chip">' + lat.toFixed(4) + ', ' + lon.toFixed(4) + '</span>');
      }
      html = chips.join('');
    }
  } catch(e){}
  albumExifCache[full] = html;
  var cur = albumList()[albumIdx];
  if (albumOpen() && cur && cur.name === it.name)
    $('#albumExif').innerHTML = html;
}

function bindAlbum(){
  // The visible \u2039 \u203a are decorative: the real links cover each screen half.
  $('#albumLinkPrev').addEventListener('click', function(e){ e.preventDefault(); albumNav(-1); });
  $('#albumLinkNext').addEventListener('click', function(e){ e.preventDefault(); albumNav(1); });
  $('#btnAlbumClose').addEventListener('click', closeAlbum);
  // Click on the empty black area (around the media) closes the album.
  $('#albumMedia').addEventListener('click', function(e){
    if (e.target === $('#albumMedia')) closeAlbum();
  });
}

document.addEventListener('keydown', function(e){
  if (!albumOpen()) return;
  if (e.key === 'Escape') closeAlbum();
  else if (e.key === 'ArrowLeft') albumNav(-1);
  else if (e.key === 'ArrowRight') albumNav(1);
});

function isImage(name){
  return /\.(png|jpe?g|gif|bmp|webp|svg|tiff?|ico|arw|cr2|cr3|nef|orf|dng|pef|raf)$/i.test(name);
}

function ext(name){
  var i = name.lastIndexOf('.');
  return i > 0 ? name.slice(i + 1).toLowerCase() : '';
}

// Human-readable size (same B/kB/MB/GB/TB scheme as the server's getFancySize).
function fancySize(n){
  var names = ['B', 'kB', 'MB', 'GB', 'TB'];
  var pos = 0;
  n = Math.max(0, Math.floor(n));
  while (n >= 1024 && pos < 4){ pos++; n = Math.round(n / 1024); }
  return n + names[pos];
}

function applyThumbSize(){
  document.documentElement.style.setProperty('--thumb-h', state.thumbSize + 'px');
}

// Mobile header icon bar: measure the natural (unclamped) width of the row
// and, when it overflows the viewport, scale it down with a transform so that
// every control stays visible no matter how small the phone is. The layout
// width is clamped to the bar's client width before measuring so a previously
// applied scale does not bias the natural-width reading.
function fitMobileBar(){
  var bar = document.querySelector('#zoneF .mbar');
  if (!bar || !isMobile()){
    if (bar) bar.style.transform = '';
    return;
  }
  var avail = bar.parentElement.clientWidth;
  if (!avail) return;
  bar.style.width = avail + 'px';   // clamp layout width so the measurement below is natural
  var nat = bar.scrollWidth;        // overflow-x:visible -> reports the full natural content width
  if (nat > avail){
    var s = avail / nat;
    bar.style.transformOrigin = 'left center';
    bar.style.transform = 'scale(' + s + ')';
  } else {
    bar.style.transform = '';
  }
  bar.style.width = '';
}

// Resolution we request from the server (2x the displayed size for HiDPI sharpness).
function thumbFetchSize(){
  return state.thumbSize * 2;
}

function sortItems(items){
  var key = state.sortKey, dir = state.sortDir;
  return items.slice().sort(function(a, b){
    var ad = a.type === 'dir', bd = b.type === 'dir';
    if (ad !== bd) return ad ? -1 : 1;   // folders always first
    var r = 0;
    if (key === 'name')      r = a.name.toLowerCase().localeCompare(b.name.toLowerCase());
    else if (key === 'date') r = (a.mtime || 0) - (b.mtime || 0);
    else if (key === 'size') r = (a.size || 0) - (b.size || 0);
    else if (key === 'type') r = ext(a.name).localeCompare(ext(b.name)) || a.name.toLowerCase().localeCompare(b.name.toLowerCase());
    return dir === 'desc' ? -r : r;
  });
}

function currentPathFromHash(){
  var p = normPath(decodeURIComponent(location.hash.replace(/^#/, '')));
  var target = '#' + encodeURIComponent(p);
  if (location.hash !== target) history.replaceState(null, '', target);
  return p;
}

function normPath(p){
  if (!p) return '/';
  if (p.charAt(0) !== '/') p = '/' + p;
  while (p.length > 1 && p.charAt(p.length - 1) === '/') p = p.slice(0, -1);
  return p;
}

function navigate(path){
  path = normPath(path);
  var target = '#' + encodeURIComponent(path);
  if (location.hash === target) loadDir(path);
  else location.hash = encodeURIComponent(path);
}

// Step back in the app's own navigation history (mobile "Back" button).
function goBack(){
  if (navIndex <= 0) return;
  navIndex--;
  navSuppressed = true;   // don't re-push this path onto the history
  navigate(navHistory[navIndex]);
}

// Enable/disable the mobile header navigation buttons.
function updateNavButtons(){
  var up = $('#btnMUp'), back = $('#btnMBack');
  if (up)   up.disabled   = !state.has_parent;
  if (back) back.disabled = (navIndex <= 0);
}

async function loadDir(path){
  state.path = path;
  state.selected = [];
  state.activeIndex = -1;
  // The album list is cached from state.items; drop it whenever the
  // directory changes, otherwise the album keeps the previous folder's
  // media (wrong count) and builds URLs with the new path + old names
  // (404s).
  albumListCache = [];
  albumIdx = -1;
  var r = await api('?act=ls&path=' + enc(path));
  if (!r || !r.ok){
    toast(r && r.error ? r.error : 'Error loading directory');
    return;
  }
  state.items = r.data.items || [];
  // Normalise the kebab-case 'can-*' keys from the ls payload into the
  // camelCase property names the rest of the UI code uses.
  var fl = r.data.flags || state.flags;
  state.flags = {
    canUpload:    !!fl['can-upload'],
    canOverwrite: !!fl['can-overwrite'],
    canArchive:   !!fl['can-archive'],
    canExecute:   !!fl['can-execute'],
    version:      fl.version || ''
  };
  state.upload_here = r.data.upload_here !== false;
  state.has_parent = !!r.data.has_parent;

  // Record the navigation history (drives the mobile "Back" button).
  var norm = normPath(r.data.path || path);
  var isBackStep = navSuppressed;
  if (!isBackStep){
    // Truncate any forward entries, then push (unless we're already there).
    if (navHistory[navIndex] !== norm){
      navHistory.length = navIndex + 1;
      navHistory.push(norm);
      navIndex = navHistory.length - 1;
    }
  }
  navSuppressed = false;
  updateNavButtons();

  renderPath(r.data.path, r.data.parent);
  renderToolbar();
  renderFiles();
  renderDetail();
  await revealPath(r.data.path);
  renderTree();
  // If the album is open, refresh it for the new directory (or close it).
  if (albumOpen()){
    if (albumList().length){ albumIdx = 0; renderAlbum(); }
    else closeAlbum();
  }
}

function makeCrumb(path){
  var parts = [ {name:'/', path:'/'} ];
  var segs = path.split('/').filter(Boolean);
  var acc = '';
  for (var i=0;i<segs.length;i++){
    acc += '/' + segs[i];
    parts.push({name:segs[i], path:acc});
  }
  return parts;
}

function renderPath(path, parent){
  $('#pathInput').value = normPath(path);
  // Second row of the mobile header: the current directory name (root = "/").
  var p = normPath(path);
  var name = (p === '/') ? '/' : p.split('/').filter(Boolean).pop();
  $('#mTitle').textContent = name;
  $('#mTitle').title = p;
}

function renderToolbar(){
  var f = state.flags;
  $('#btnUpload').disabled = !(f.canUpload && state.upload_here);
  $('#btnNewFolder').disabled = !(f.canUpload && state.upload_here);
  $('#btnDelete').disabled = !f.canOverwrite;
  updateSelectionButtons();
}

function setView(v){
  state.view = v;
  $('#filesGrid').className = v === 'list' ? 'view-list' : 'view-grid';
  $('#btnGrid').style.background = v==='grid' ? '#23303f' : '';
  $('#btnList').style.background = v==='list' ? '#23303f' : '';
  renderFiles();
}

function toggleFullscreen(){
  if (!document.fullscreenElement) document.documentElement.requestFullscreen();
  else document.exitFullscreen();
}

// ---------------- TREE ----------------
async function loadTree(path){
  var r = await api('?act=tree&path=' + enc(path));
  if (r && r.ok) treeCache[path] = r.data.nodes || [];
}
async function revealPath(path){
  if (path === '/' || !path) return;
  var segs = path.split('/').filter(Boolean);
  var acc = '';
  for (var i=0;i<segs.length;i++){
    acc += '/' + segs[i];
    expanded[acc] = true;
    var p = acc.lastIndexOf('/') === 0 ? '/' : acc.slice(0, acc.lastIndexOf('/'));
    if (!treeCache[p]) await loadTree(p);
  }
}
function renderTreeLevel(container, path, depth){
  var nodes = treeCache[path] || [];
  nodes.forEach(function(node){
    var row = document.createElement('div');
    row.className = 'tree-row' + (node.path === state.path ? ' active' : '');
    row.style.paddingLeft = (8 + depth*14) + 'px';
    var tw = document.createElement('span');
    tw.className = 'tw';
    tw.textContent = node.has_children ? (expanded[node.path] ? '▾' : '▸') : '';
    tw.onclick = function(ev){ ev.stopPropagation(); toggleTree(node.path); };
    var ic = document.createElement('i'); ic.className = 'ic ic-folder';
    var lbl = document.createElement('span'); lbl.className = 'lbl'; lbl.textContent = node.name;
    row.appendChild(tw); row.appendChild(ic); row.appendChild(lbl);
    row.onclick = function(){ navigate(node.path); };
    container.appendChild(row);
    if (expanded[node.path]){
      var box = document.createElement('div');
      container.appendChild(box);
      renderTreeLevel(box, node.path, depth+1);
    }
  });
}
function renderTree(){
  var c = $('#tree'); c.innerHTML = '';
  var root = document.createElement('div');
  c.appendChild(root);
  renderTreeLevel(root, '/', 0);
}
async function toggleTree(path){
  if (expanded[path]) delete expanded[path];
  else { expanded[path] = true; if (!treeCache[path]) await loadTree(path); }
  renderTree();
}

// ---------------- FILES ----------------
function itemPath(name){ return state.path === '/' ? '/' + name : state.path + '/' + name; }
function getItem(name){ return state.items.find(function(i){ return i.name === name; }); }
function isSelected(name){ return state.selected.indexOf(name) >= 0; }
function escapeHtml(s){ return String(s).replace(/[&<>\"]/g, function(c){ return {'&':'&amp;','<':'&lt;','>':'&gt;','\"':'&quot;'}[c]; }); }

// Truncate long names for the grid view: first 20 chars + "..." + the last
// 3 chars of the extension (e.g. "very_long_name_here.png" -> "very_long_nam...png").
// Names without an extension only get the leading 20 chars + "...".
// The full name stays available in the title tooltip.
function shortName(name){
  if (name.length <= 20) return name;
  var i = name.lastIndexOf('.');
  if (i > 0){
    var ext = name.slice(i + 1);
    return name.slice(0, 20) + '...' + (ext.length > 3 ? '.' + ext.slice(-3) : ext);
  }
  return name.slice(0, 20) + '...';
}

function renderFiles(){
  var grid = $('#filesGrid');
  grid.innerHTML = '';
  var q = state.filter.toLowerCase();

  var items = sortItems(state.items.filter(function(it){ return !q || it.name.toLowerCase().indexOf(q) >= 0; }));
  $('#emptyMsg').style.display = items.length ? 'none' : 'block';

  items.forEach(function(it){
    var el = document.createElement('div');
    el.className = 'item' + (it.type === 'dir' ? ' is-dir' : ' is-file') + (isSelected(it.name) ? ' selected' : '');
    el.dataset.name = it.name;

    var thumb = document.createElement('div');
    thumb.className = 'thumb';
    if (it.type === 'dir'){
      thumb.innerHTML = '<i class="ic ic-folder"></i>';
    } else if (isImage(it.name) || isVideo(it.name)){
      var full = itemPath(it.name);
      var key = full + '@' + thumbFetchSize();
      var img = document.createElement('img');
      img.alt = it.name;
      // A loaded 2x thumbnail can be wider than the placeholder it replaces;
      // re-measure the page-wide card width once the real image is in.
      img.addEventListener('load', function(){ measureGridWidths(); });
      if (thumbCache[key]){
        img.src = thumbCache[key];
      } else {
        img.dataset.thumbKey = key;   // fetched sequentially by thumbWorker
      }
      thumb.appendChild(img);
    } else {
      thumb.innerHTML = '<i class=\"ic ic-file\"></i>';
    }

    var nm = document.createElement('div');
    nm.className = 'name';
    // Grid view: truncated (20 + "..." + ext tail) so wide names don't
    // stretch the card. List view keeps the full name.
    nm.textContent = state.view === 'grid' ? shortName(it.name) : it.name;
    nm.title = it.name;

    el.appendChild(thumb);
    el.appendChild(nm);

    if (state.view === 'list'){
      var meta = document.createElement('div');
      meta.className = 'meta';
      var sz = document.createElement('span');
      sz.textContent = it.type === 'dir' ? '' : it.size_fancy;
      var mt = document.createElement('span');
      var d = new Date(it.mtime * 1000);
      mt.textContent = d.toLocaleString();
      var pm = document.createElement('span');
      pm.textContent = it.perms;
      pm.title = 'uid:gid ' + it.uid + ':' + it.gid;
      if (it.warning) pm.style.color = 'var(--danger)';
      meta.appendChild(sz); meta.appendChild(mt); meta.appendChild(pm);
      el.appendChild(meta);
    }

    el.addEventListener('click', function(ev){
      if (ev.shiftKey){ selectRange(it.name, ev); return; }
      if (isMobile()){
        // Single tap opens the item and shows the context menu directly.
        select(it.name, false);
        showCtxMenu(ev.clientX, ev.clientY);
        openItem(it.name);
        return;
      }
      select(it.name, ev.ctrlKey || ev.metaKey);
    });
    el.addEventListener('dblclick', function(){ openItem(it.name); });
    el.addEventListener('contextmenu', function(ev){
      ev.preventDefault();
      if (!isSelected(it.name)) select(it.name, false);
      showCtxMenu(ev.clientX, ev.clientY);
    });

    grid.appendChild(el);
  });

  if (state.view === 'grid') measureGridWidths();
  clampActiveIndex();
  updateActiveClass();
  scheduleThumbs();
}

// Grid view: every card is width:max-content, so a wide image or a long name
// stretches only that card and the grid looks ragged. Measure the widest card
// of the WHOLE page (not just the row) and lock that width onto all cards so
// the whole grid aligns. Re-runs after each thumbnail loads, since a loaded
// image can be wider than the placeholder it replaces.
//
// Before measuring we must clear the previously forced inline width on every
// card: otherwise the forced width clamps offsetWidth and the measurement is
// stuck at the (possibly too-small) value from the last run. Temporarily
// restore width:max-content, read the natural widths, then re-apply the new
// maximum.
function measureGridWidths(){
  if (state.view !== 'grid') return;
  var items = $$('#filesGrid .item');
  if (!items.length) return;
  // Clear the previously locked width so each card reports its natural
  // max-content width (thumbnail may have loaded since the last measurement).
  items.forEach(function(el){ el.style.width = ''; });
  var max = 0;
  items.forEach(function(el){
    var w = el.offsetWidth;
    if (w > max) max = w;
  });
  if (!max) return;
  items.forEach(function(el){ el.style.width = max + 'px'; });
}

function scheduleThumbs(){
  $$('#filesGrid img[data-thumb-key]').forEach(function(img){
    var key = img.getAttribute('data-thumb-key');
    if (!key || thumbCache[key] || thumbPending[key] || thumbQueued[key]) return;
    thumbQueued[key] = true;
    thumbQueue.push(key);
  });
  while (thumbWorkersRunning < THUMB_CONCURRENCY && thumbQueue.length)
    thumbWorker();
}

async function thumbWorker(){
  thumbWorkersRunning++;
  while (thumbQueue.length){
    var key = thumbQueue.shift();
    delete thumbQueued[key];
    if (thumbCache[key]) continue;

    var sep = key.lastIndexOf('@');
    var path = key.slice(0, sep);
    var size = key.slice(sep + 1);

    thumbPending[key] = true;
    try {
      var r = await api('?act=thumb&file=' + enc(path) + '&s=' + size);
      if (r && r.ok && r.data && r.data.b64){
        thumbCache[key] = 'data:' + (r.data.mime || 'image/jpeg') + ';base64,' + r.data.b64;
      }
    } catch(e){}
    delete thumbPending[key];

    // Apply to any (possibly re-rendered) <img> waiting on this exact key.
    $$('#filesGrid img[data-thumb-key]').forEach(function(img){
      if (img.getAttribute('data-thumb-key') === key){
        if (thumbCache[key]){
          img.src = thumbCache[key];
          img.removeAttribute('data-thumb-key');
        } else {
          img.parentNode.innerHTML = '<i class="ic ic-file"></i>';
        }
      }
    });
  }
  thumbWorkersRunning--;
  if (thumbQueue.length && thumbWorkersRunning < THUMB_CONCURRENCY)
    thumbWorker();   // queue still has work and we're below the cap: refill
}


function select(name, additive){
  if (additive){
    var i = state.selected.indexOf(name);
    if (i >= 0) state.selected.splice(i, 1);
    else state.selected.push(name);
  } else {
    state.selected = [name];
  }
  syncActiveFromSelection();
  updateSelectionButtons();
  renderFiles();
  renderDetail();
}

// Names in the exact order the items are currently rendered (filter + sort applied).
function visibleNames(){
  var q = state.filter.toLowerCase();
  return state.items.filter(function(it){ return !q || it.name.toLowerCase().indexOf(q) >= 0; })
    .map(function(it){ return it.name; });
}

// All rendered names between two names (inclusive), in render order.
function rangeNames(a, b){
  var names = visibleNames();
  var ia = names.indexOf(a), ib = names.indexOf(b);
  if (ia < 0 || ib < 0) return [a];
  if (ia > ib){ var t = ia; ia = ib; ib = t; }
  return names.slice(ia, ib + 1);
}

// Update just the .selected classes + toolbar buttons without a full re-render.
function applySelectionClass(){
  var sel = {};
  state.selected.forEach(function(n){ sel[n] = true; });
  $$('#filesGrid .item').forEach(function(el){
    el.classList.toggle('selected', !!sel[el.dataset.name]);
  });
  updateSelectionButtons();
}

// Shift+click: extend the selection from the last selected item to the clicked one.
function selectRange(name, ev){
  var names = visibleNames();
  var anchorName = null;
  if (state.selected.length){
    anchorName = state.selected[state.selected.length - 1];
    if (names.indexOf(anchorName) < 0) anchorName = state.selected[0];
  }
  if (!anchorName){ state.selected = [name]; }
  else {
    state.selected = rangeNames(anchorName, name);
    if (ev.ctrlKey || ev.metaKey){
      // Add the range to the existing selection instead of replacing it.
      state.selected = state.selected.concat(rangeNames(anchorName, name).filter(function(n){ return state.selected.indexOf(n) < 0; }));
    }
  }
  syncActiveFromSelection();
  applySelectionClass();
  renderDetail();
}

function updateSelectionButtons(){
  var n = state.selected.length;
  $('#selActions').classList.toggle('show', n > 0);

  // The direct "Download" button only makes sense for a single file.
  // For a folder, or for several items, the user must pick a format via
  // the archive buttons, so hide it (the archive entries stay available).
  var singleFile = (n === 1) && !!getItem(state.selected[0]) && getItem(state.selected[0]).type === 'file';
  $('#btnDownload').style.display = singleFile ? '' : 'none';
  $('#btnDownload').disabled = (!singleFile);
  $('#btnDelete').disabled = (n === 0 || !state.flags.canOverwrite);
}

function openItem(name){
  var it = getItem(name);
  var full = itemPath(name);
  if (!it) return;
  if (it.type === 'dir') navigate(full);
  else if (state.flags.canExecute && it.executable) window.open('?act=exec&file=' + enc(full), '_blank');
  else if (isMobile() && it.type === 'file' && (isImage(it.name) || isVideo(it.name))) openAlbum(it.name);
  else window.open('?act=download&file=' + enc(full), '_blank');
}

// ---------------- DETAILS PANEL ----------------
function addDetailRows(container, pairs, heading){
  if (!pairs || !pairs.length) return;
  var h2 = document.createElement('h2'); h2.textContent = heading;
  container.appendChild(h2);
  var tbl = document.createElement('table');
  pairs.forEach(function(p){
    var tr = document.createElement('tr');
    var k = document.createElement('td'); k.className = 'k'; k.textContent = p[0];
    var v = document.createElement('td'); v.className = 'v';
    v.textContent = (p[1] === null || p[1] === undefined) ? '' : String(p[1]);
    tr.appendChild(k); tr.appendChild(v);
    tbl.appendChild(tr);
  });
  container.appendChild(tbl);
}

// Stats of the current folder shown when nothing is selected.
function renderFolderInfo(zoneE){
  var items = state.items;
  var dirs = 0, files = 0, size = 0;
  items.forEach(function(it){
    if (it.type === 'dir') dirs++;
    else { files++; size += (it.size || 0); }
  });
  zoneE.innerHTML = '';
  var h = document.createElement('h3'); h.textContent = 'Folder Information';
  zoneE.appendChild(h);
  var rows = [['Items', items.length], ['Folders', dirs], ['Files', files]];
  if (files) rows.push(['Total size', fancySize(size) + ' (' + size + ' bytes)']);
  var box = document.createElement('div'); box.className = 'exif dirinfo';
  zoneE.appendChild(box);
  addDetailRows(box, rows, 'Directory');
}

function renderDetail(){
  var zoneE = $('#detailContent');
  var mid = $('#zoneMid');
  var n = state.selected.length;

  // The panel is always visible, so no need to toggle has-detail class.
  if (n === 0){ renderFolderInfo(zoneE); return; }
  if (n > 1){
    zoneE.innerHTML = '<div class="empty">' + n + ' items selected</div>';
    return;
  }

  var name = state.selected[0];
  var it = getItem(name);
  if (!it){ zoneE.innerHTML = '<div class="empty">No selection</div>'; return; }

  var full = itemPath(name);
  zoneE.innerHTML = '';
  var h = document.createElement('h3'); h.textContent = name;
  zoneE.appendChild(h);

  var isImg = (it.type === 'file' && (isImage(it.name) || isVideo(it.name)));

  var dt = null;
  if (isImg){
    dt = document.createElement('div'); dt.className = 'd-thumb';
    var img = document.createElement('img'); img.alt = name;
    api('?act=thumb&file=' + enc(full) + '&s=512').then(function(r){
      if (r && r.ok && r.data && r.data.b64)
        img.src = 'data:' + (r.data.mime || 'image/png') + ';base64,' + r.data.b64;
    });
    // Click on the detail thumbnail opens the album (same as the gallery button).
    img.addEventListener('click', function(){ $('#btnAlbum').click(); });
    dt.appendChild(img);
    zoneE.appendChild(dt);
  }

  addDetailRows(zoneE, [
    ['Type', it.type === 'dir' ? 'Folder' : 'File'],
    ['Size', it.type === 'dir' ? '-' : (it.size_fancy + ' (' + it.size + ' bytes)')],
    ['Modified', new Date(it.mtime * 1000).toLocaleString()],
    ['Permissions', it.perms],
    ['Owner', 'uid:gid ' + it.uid + ':' + it.gid]
  ], 'File');

  if (isImg){
    var exifBox = document.createElement('div'); exifBox.className = 'exif';
    zoneE.appendChild(exifBox);
    api('?act=detail&file=' + enc(full)).then(function(r){
      if (!r || !r.ok || !r.data) return;
      var imgRows = [];
      if (r.data.mime) imgRows.push(['MIME', r.data.mime]);
      if (r.data.width || r.data.height) imgRows.push(['Dimensions', (r.data.width || '?') + ' \u00d7 ' + (r.data.height || '?')]);
      addDetailRows(exifBox, imgRows, 'Image');
      if (r.data.exif){
        var keys = Object.keys(r.data.exif);
        if (keys.length) addDetailRows(exifBox, keys.map(function(k){ return [k, r.data.exif[k]]; }), 'EXIF');
      }
    });
  }
}

// ---------------- CONTEXT MENU ----------------
var ctx = $('#ctxMenu');
function hideCtx(){ ctx.style.display = 'none'; }
function sep(){ var s = document.createElement('div'); s.className = 'sep'; return s; }
function mi(icon, label, disabled, fn){
  var d = document.createElement('div');
  d.className = 'mi' + (disabled ? ' disabled' : '');
  if (icon){ d.innerHTML = '<i class=\"ic ' + icon + '\"></i>'; }
  var s = document.createElement('span'); s.textContent = label;
  d.appendChild(s);
  if (!disabled && fn){ d.onclick = function(){ hideCtx(); fn(); }; }
  return d;
}
function showCtxMenu(x, y){
  hideCtx();
  ctx.innerHTML = '';
  var n = state.selected.length;
  var canWrite = state.flags.canUpload && state.upload_here;

  if (n === 0){
    ctx.appendChild(mi('ic-newfolder', 'New folder', !canWrite, doNewFolder));
    ctx.appendChild(mi('ic-upload', 'Upload here', !canWrite, doUpload));
    ctx.appendChild(sep());
    ctx.appendChild(mi('ic-refresh', 'Refresh', false, doRefresh));
  } else {
    if (n === 1) ctx.appendChild(mi('ic-folder', 'Open', false, function(){ openItem(state.selected[0]); }));

    var sel0 = n === 1 ? getItem(state.selected[0]) : null;
    // Direct "Download" only applies to a single file. A folder or a
    // multi-selection is downloaded through an archive, so the user picks
    // the format below (no implicit tar.gz).
    if (n === 1 && sel0 && sel0.type === 'file')
      ctx.appendChild(mi('ic-download', 'Download', false, doDownloadSelected));

    if (state.flags.canArchive){
      ctx.appendChild(mi('ic-download', 'Archive .tar.gz', false, function(){ doArchive('tar.gz'); }));
      ctx.appendChild(mi('ic-download', 'Archive .tar', false, function(){ doArchive('tar'); }));
      ctx.appendChild(mi('ic-download', 'Archive .zip', false, function(){ doArchive('zip'); }));
    }

    if (n === 1){
      if (state.flags.canExecute && sel0 && sel0.type === 'file' && sel0.executable){
        ctx.appendChild(mi('ic-file', 'Execute', false, function(){ window.open('?act=exec&file=' + enc(itemPath(state.selected[0])), '_blank'); }));
      }
    }
    ctx.appendChild(sep());
    ctx.appendChild(mi('ic-delete', 'Delete', !state.flags.canOverwrite, doDeleteSelected));
  }

  ctx.style.display = 'block';
  ctx.style.left = x + 'px';
  ctx.style.top = y + 'px';
  var w = ctx.offsetWidth, h = ctx.offsetHeight;
  if (x + w > window.innerWidth) ctx.style.left = (x - w) + 'px';
  if (y + h > window.innerHeight) ctx.style.top = (y - h) + 'px';
}

// ---------------- ACTIONS ----------------
function doRefresh(){ loadDir(state.path); }
function selectedPaths(){ return state.selected.map(itemPath); }
function doDownloadSelected(){
  // Only a single file reaches here: folders and multi-selections are
  // downloaded through an archive where the user explicitly picks the format.
  var sel = state.selected[0];
  if (!sel) return;
  window.location = '?act=download&file=' + enc(itemPath(sel)) + '&dl=1';
}
function doArchive(fmt){
  var files = state.selected.map(itemPath).join(',');
  window.location = '?act=archive&files=' + enc(files) + '&fmt=' + fmt;
}
function doUpload(){
  if (!(state.flags.canUpload && state.upload_here)){ toast('Uploads disabled here'); return; }
  $('#fileInput').click();
}
async function uploadFiles(fileList){
  if (!(state.flags.canUpload && state.upload_here)){ toast('Uploads disabled here'); return; }
  var fd = new FormData();
  for (var i=0;i<fileList.length;i++) fd.append('file', fileList[i]);
  var r = await api('?act=upload&path=' + enc(state.path), { method:'POST', body: fd });
  if (r && r.ok && r.data && r.data.uploaded){ toast('Upload complete'); loadDir(state.path); }
  else toast(r && r.error ? r.error : 'Upload failed');
}
function doNewFolder(){
  if (!(state.flags.canUpload && state.upload_here)){ toast('Creation disabled here'); return; }
  openDialog('New folder', [{ key:'name', placeholder:'Folder name' }], function(values){
    var name = values.name.trim();
    if (!name) return;
    api('?act=mkdir&path=' + enc(state.path) + '&name=' + enc(name), { method:'POST' }).then(function(r){
      if (r && r.ok){ toast('Folder created'); loadDir(state.path); }
      else toast(r && r.error ? r.error : 'Could not create folder');
    });
  });
}
function doDeleteSelected(){
  if (!confirm('Delete ' + state.selected.length + ' item(s)?')) return;
  var files = state.selected.map(itemPath).join(',');
  api('?act=delete&files=' + enc(files), { method:'POST' }).then(function(r){
    if (r && r.ok){ toast('Deleted'); state.selected = []; loadDir(state.path); }
    else toast(r && r.error ? r.error : 'Delete failed');
  });
}

// ---------------- MODAL ----------------
function openDialog(title, fields, onSubmit){
  var dlg = $('#dialog');
  dlg.innerHTML = '';
  var h = document.createElement('h3'); h.textContent = title;
  dlg.appendChild(h);
  var inputs = {};
  fields.forEach(function(f){
    var inp = document.createElement('input');
    inp.type = f.type || 'text';
    inp.placeholder = f.placeholder || '';
    dlg.appendChild(inp);
    inputs[f.key] = inp;
  });
  var actions = document.createElement('div'); actions.className = 'actions';
  var cancel = document.createElement('button'); cancel.className = 'btn'; cancel.textContent = 'Cancel';
  cancel.onclick = closeOverlay;
  var ok = document.createElement('button'); ok.className = 'btn primary'; ok.textContent = 'OK';
  ok.onclick = function(){
    var vals = {};
    Object.keys(inputs).forEach(function(k){ vals[k] = inputs[k].value; });
    closeOverlay();
    onSubmit(vals);
  };
  actions.appendChild(cancel); actions.appendChild(ok);
  dlg.appendChild(actions);
  $('#overlay').classList.add('open');
  var first = fields[0]; inputs[first.key].focus();
  inputs[first.key].addEventListener('keydown', function(e){ if (e.key === 'Enter') ok.click(); });
}
function closeOverlay(){ $('#overlay').classList.remove('open'); }

// ---------------- RUBBER-BAND SELECTION ----------------
// Drag on empty space (or starting from an item) selects everything in the
// rectangle, or everything between the anchor item and the item under the
// cursor when the drag ends over an item.
var drag = null;

function dragItemAt(clientX, clientY){
  var el = document.elementFromPoint(clientX, clientY);
  if (!el) return null;
  var item = el.closest ? el.closest('#filesGrid .item') : null;
  return item ? item.dataset.name : null;
}

function rectIntersects(rx, ry, rw, rh, r){
  return rx < r.right && rx + rw > r.left && ry < r.bottom && ry + rh > r.top;
}

function applyDragSelection(anchorName, endName, shiftHeld, rect){
  var base = shiftHeld ? drag.baseSelection : [];
  var sel;
  if (anchorName && endName){
    // Range mode: everything between anchor and end (render order).
    sel = rangeNames(anchorName, endName).filter(function(n){ return base.indexOf(n) < 0; });
    sel = base.concat(sel);
  } else {
    // Rectangle mode: intersect with the visible items.
    var picked = [];
    $$('#filesGrid .item').forEach(function(el){
      if (rectIntersects(rect.x, rect.y, rect.w, rect.h, el.getBoundingClientRect()))
        picked.push(el.dataset.name);
    });
    sel = picked.filter(function(n){ return base.indexOf(n) < 0; });
    sel = base.concat(sel);
  }
  state.selected = sel;
  applySelectionClass();
}

function updateRubberBand(sx, sy, cx, cy){
  var zb = $('#zoneD').getBoundingClientRect();
  var band = $('#rubberBand');
  // Convert viewport coords to #zoneD-local coords (scroll is handled since
  // the band lives inside the scrolled content).
  var x1 = Math.min(sx, cx) - zb.left, x2 = Math.max(sx, cx) - zb.left;
  var y1 = Math.min(sy, cy) - zb.top,  y2 = Math.max(sy, cy) - zb.top;
  band.style.left = x1 + 'px';
  band.style.top = y1 + 'px';
  band.style.width = (x2 - x1) + 'px';
  band.style.height = (y2 - y1) + 'px';
  band.style.display = 'block';
}

function hideRubberBand(){
  $('#rubberBand').style.display = 'none';
}

function onDragMove(ev){
  if (!drag) return;
  if (!drag.started && (Math.abs(ev.clientX - drag.sx) > 4 || Math.abs(ev.clientY - drag.sy) > 4)){
    drag.started = true;
  }
  if (!drag.started) return;

  var zb = $('#zoneD').getBoundingClientRect();
  var cx = Math.min(Math.max(ev.clientX, zb.left), zb.right);
  var cy = Math.min(Math.max(ev.clientY, zb.top), zb.bottom);
  updateRubberBand(drag.sx, drag.sy, cx, cy);

  var rect = {
    x: Math.min(drag.sx, cx), y: Math.min(drag.sy, cy),
    w: Math.abs(cx - drag.sx), h: Math.abs(cy - drag.sy)
  };
  var endName = dragItemAt(ev.clientX, ev.clientY);
  applyDragSelection(drag.anchorName, endName, drag.shiftHeld, rect);
}

function onDragEnd(){
  if (!drag) return;
  if (drag.started){
    renderDetail();
  }
  drag = null;
  hideRubberBand();
  document.body.style.userSelect = '';
  document.body.style.cursor = '';
}

function bindSelection(){
  var zoneD = $('#zoneD');

  zoneD.addEventListener('mousedown', function(ev){
    if (ev.button !== 0 || drag) return;
    var item = ev.target.closest ? ev.target.closest('#filesGrid .item') : null;
    var onItem = !!(item && item.dataset.name);
    var shift = ev.shiftKey;
    var ctrl = ev.ctrlKey || ev.metaKey;

    // If it's a normal click on an item without modifiers, let the click handler deal with it.
    if (onItem && !shift && !ctrl){
      // Just start tracking in case it turns into a drag.
      drag = {
        sx: ev.clientX, sy: ev.clientY, started: false,
        anchorName: item.dataset.name, shiftHeld: false,
        baseSelection: state.selected.slice()
      };
      return;
    }

    // If it's a ctrl/cmd click on an item, let the click handler deal with it.
    if (onItem && !shift && ctrl){
      // Just start tracking in case it turns into a drag.
      drag = {
        sx: ev.clientX, sy: ev.clientY, started: false,
        anchorName: item.dataset.name, shiftHeld: false,
        baseSelection: state.selected.slice()
      };
      return;
    }

    // If it's a shift click on an item, let the click handler deal with it.
    if (onItem && shift && !ctrl){
      // Start a range drag from the last selected item.
      var anchorName = state.selected.length ? state.selected[state.selected.length - 1] : item.dataset.name;
      drag = {
        sx: ev.clientX, sy: ev.clientY, started: false,
        anchorName: anchorName, shiftHeld: true,
        baseSelection: state.selected.slice()
      };
      ev.preventDefault();
      return;
    }

    // Empty area: plain click clears; a drag starts the rubber band.
    var cleared = state.selected.length > 0;
    state.selected = [];
    if (cleared){ applySelectionClass(); renderDetail(); }

    var anchorName = dragItemAt(ev.clientX, ev.clientY);
    drag = {
      sx: ev.clientX, sy: ev.clientY, started: false,
      anchorName: anchorName, shiftHeld: shift,
      baseSelection: shift ? state.selected.slice() : []
    };
    ev.preventDefault();
  });

  // Prevent click events from interfering with drag operations.
  zoneD.addEventListener('click', function(ev){
    if (drag && drag.started){
      ev.stopPropagation();
      ev.preventDefault();
    }
  }, true);

  document.addEventListener('mousemove', onDragMove);
  document.addEventListener('mouseup', onDragEnd);
  document.addEventListener('mouseleave', onDragEnd);
}

// ---------------- DRAG & DROP UPLOAD ----------------
// Only react to drags that carry files; anything else (e.g. text selection
// or dragging an item with the rubber band) falls through untouched.
function dragHasFiles(ev){
  return !!(ev.dataTransfer && ev.dataTransfer.types &&
    Array.prototype.indexOf.call(ev.dataTransfer.types, 'Files') >= 0);
}

function bindDropUpload(){
  var overlay = $('#dropOverlay');

  window.addEventListener('dragenter', function(ev){
    if (!dragHasFiles(ev)) return;
    ev.preventDefault();   // needed to allow dropping
    overlay.classList.add('open');
  });

  window.addEventListener('dragover', function(ev){
    if (!dragHasFiles(ev)) return;
    ev.preventDefault();   // without this the browser navigates away
  });

  window.addEventListener('dragleave', function(ev){
    if (!dragHasFiles(ev)) return;
    if (ev.relatedTarget) return;   // moved inside another element
    overlay.classList.remove('open');
  });

  window.addEventListener('drop', function(ev){
    if (!dragHasFiles(ev)) return;
    ev.preventDefault();
    overlay.classList.remove('open');
    if (ev.dataTransfer && ev.dataTransfer.files && ev.dataTransfer.files.length)
      uploadFiles(ev.dataTransfer.files);
  });
}

// ---------------- KEYBOARD SHORTCUTS (PC mode only) ----------------
// The keyboard cursor (activeIndex) points into visibleNames(). Arrow keys
// move it; Enter opens; Space toggles selection; Delete removes; etc.
// On mobile the handler returns early so nothing interferes.

function visibleNamesForActive(){
  var q = state.filter.toLowerCase();
  return state.items.filter(function(it){ return !q || it.name.toLowerCase().indexOf(q) >= 0; })
    .map(function(it){ return it.name; });
}

function clampActiveIndex(){
  var names = visibleNamesForActive();
  if (state.activeIndex < 0 || state.activeIndex >= names.length)
    state.activeIndex = names.length ? 0 : -1;
}

function syncActiveFromSelection(){
  if (state.selected.length){
    var names = visibleNamesForActive();
    var anchor = state.selected[state.selected.length - 1];
    var idx = names.indexOf(anchor);
    if (idx >= 0) state.activeIndex = idx;
    else { var first = names.indexOf(state.selected[0]); if (first >= 0) state.activeIndex = first; }
  }
}

function updateActiveClass(){
  var names = visibleNamesForActive();
  var activeName = state.activeIndex >= 0 ? names[state.activeIndex] : null;
  $$('#filesGrid .item').forEach(function(el){
    el.classList.toggle('active', el.dataset.name === activeName);
  });
}

function moveActive(dir, ev){
  var names = visibleNamesForActive();
  if (!names.length) return;
  var next = state.activeIndex + dir;
  if (next < 0) next = 0;
  if (next >= names.length) next = names.length - 1;
  state.activeIndex = next;
  var name = names[state.activeIndex];
  if (ev && ev.shiftKey){
    var anchorName = state.selected.length ? state.selected[state.selected.length - 1] : name;
    state.selected = rangeNames(anchorName, name);
  } else if (ev && (ev.ctrlKey || ev.metaKey)){
    var i = state.selected.indexOf(name);
    if (i >= 0) state.selected.splice(i, 1);
    else state.selected.push(name);
  } else {
    state.selected = [name];
  }
  applySelectionClass();
  renderDetail();
  updateActiveClass();
  var el = document.querySelector('#filesGrid .item[data-name="' + CSS.escape(name) + '"]');
  if (el) el.scrollIntoView({ block: 'nearest' });
}

function bindKeyboard(){
  document.addEventListener('keydown', function(e){
    if (albumOpen()) return;
    var tag = (e.target.tagName || '').toLowerCase();
    if (tag === 'input' || tag === 'select' || tag === 'textarea') return;
    if (isMobile()) return;

    var ctrl = e.ctrlKey || e.metaKey;
    var shift = e.shiftKey;
    var key = e.key;

    if (key === 'ArrowDown' || key === 'ArrowRight'){
      e.preventDefault();
      moveActive(1, e);
      return;
    }
    if (key === 'ArrowUp' || key === 'ArrowLeft'){
      e.preventDefault();
      moveActive(-1, e);
      return;
    }

    if (key === 'Enter'){
      var names = visibleNamesForActive();
      if (state.activeIndex >= 0 && names[state.activeIndex]){
        e.preventDefault();
        openItem(names[state.activeIndex]);
      }
      return;
    }

    if (key === ' '){
      e.preventDefault();
      var nm = visibleNamesForActive();
      if (state.activeIndex >= 0 && nm[state.activeIndex]){
        var name = nm[state.activeIndex];
        var i = state.selected.indexOf(name);
        if (i >= 0) state.selected.splice(i, 1);
        else state.selected.push(name);
        applySelectionClass();
        renderDetail();
      }
      return;
    }

    if (key === 'Delete' || key === 'Backspace'){
      e.preventDefault();
      if (state.selected.length && state.flags.canOverwrite){
        doDeleteSelected();
      } else if (key === 'Backspace' && !state.selected.length){
        navigate(upPath());
      }
      return;
    }

    if (key === 'Home'){
      e.preventDefault();
      var names0 = visibleNamesForActive();
      if (names0.length){
        state.activeIndex = 0;
        state.selected = [names0[0]];
        applySelectionClass();
        renderDetail();
        updateActiveClass();
        var el0 = document.querySelector('#filesGrid .item[data-name="' + CSS.escape(names0[0]) + '"]');
        if (el0) el0.scrollIntoView({ block: 'start' });
      }
      return;
    }

    if (key === 'End'){
      e.preventDefault();
      var namesE = visibleNamesForActive();
      if (namesE.length){
        state.activeIndex = namesE.length - 1;
        state.selected = [namesE[state.activeIndex]];
        applySelectionClass();
        renderDetail();
        updateActiveClass();
        var elE = document.querySelector('#filesGrid .item[data-name="' + CSS.escape(namesE[state.activeIndex]) + '"]');
        if (elE) elE.scrollIntoView({ block: 'end' });
      }
      return;
    }

    if (ctrl && (key === 'a' || key === 'A')){
      e.preventDefault();
      state.selected = visibleNamesForActive().slice();
      applySelectionClass();
      renderDetail();
      return;
    }

    if (ctrl && shift && (key === 'd' || key === 'D')){
      e.preventDefault();
      if (state.selected.length && state.flags.canArchive) doArchive('tar.gz');
      return;
    }

    if (ctrl && (key === 'd' || key === 'D')){
      e.preventDefault();
      if (state.selected.length) doDownloadSelected();
      return;
    }

    if (ctrl && (key === 'x' || key === 'X')){
      e.preventDefault();
      if (state.selected.length && state.flags.canArchive) doArchive('zip');
      return;
    }

    if (ctrl && (key === 'l' || key === 'L')){
      e.preventDefault();
      $('#pathInput').focus();
      $('#pathInput').select();
      return;
    }
  });

  // Single-key shortcuts (no modifiers) get a second listener so the
  // Ctrl/Shift combinations above can short-circuit them cleanly.
  document.addEventListener('keydown', function(e){
    if (albumOpen()) return;
    var tag = (e.target.tagName || '').toLowerCase();
    if (tag === 'input' || tag === 'select' || tag === 'textarea') return;
    if (isMobile()) return;
    if (e.ctrlKey || e.metaKey || e.shiftKey) return;

    var names;
    switch (e.key){
      case 'r': e.preventDefault(); doRefresh(); break;
      case 'n': e.preventDefault(); doNewFolder(); break;
      case 'u': e.preventDefault(); doUpload(); break;
      case 'a': e.preventDefault(); openAlbum(); break;
      case 'f': e.preventDefault(); $('#filterInput').focus(); break;
      case 'p': e.preventDefault(); $('#pathInput').focus(); $('#pathInput').select(); break;
      case 'g': e.preventDefault(); setView('grid'); break;
      case 'l': e.preventDefault(); setView('list'); break;
      case 'F': e.preventDefault(); toggleFullscreen(); break;
      case 'Escape':
        hideCtx();
        if (state.selected.length){
          state.selected = [];
          applySelectionClass();
          renderDetail();
        }
        break;
    }
  });
}

// ---------------- BIND + INIT ----------------
function upPath(){
  var p = state.path;
  if (p === '/' || p === '') return '/';
  var i = p.lastIndexOf('/');
  if (i <= 0) return '/';
  return p.slice(0, i);
}

function bind(){
  $('#btnBack').onclick = function(){ history.back(); };
  $('#btnUp').onclick = function(){ navigate(upPath()); };
  $('#btnRefresh').onclick = doRefresh;
  $('#btnNewFolder').onclick = doNewFolder;
  $('#btnUpload').onclick = doUpload;
  $('#btnDownload').onclick = doDownloadSelected;
  $('#btnDelete').onclick = doDeleteSelected;
  // Mobile header navigation buttons (hidden on desktop).
  $('#btnMUp').onclick = function(){ if (state.has_parent) navigate(upPath()); };
  $('#btnMRefresh').onclick = doRefresh;
  $('#btnMBack').onclick = goBack;
  $('#btnGrid').onclick = function(){ setView('grid'); };
  $('#btnList').onclick = function(){ setView('list'); };
  $('#btnAlbum').onclick = function(){ openAlbum(); };
  $('#btnFullscreen').onclick = toggleFullscreen;
  $('#btnGo').onclick = function(){ navigate($('#pathInput').value); };
  $('#pathInput').addEventListener('keydown', function(e){ if (e.key === 'Enter') navigate($('#pathInput').value); });
  $('#filterInput').addEventListener('input', function(){ state.filter = this.value; renderFiles(); });

  $('#sizeSelect').addEventListener('change', function(){
    state.thumbSize = parseInt(this.value, 10) || 72;
    applyThumbSize();
    renderFiles();   // regenerate thumbnails at the new resolution
  });

  $('#sortSelect').addEventListener('change', function(){
    var kv = this.value.split(':');
    state.sortKey = kv[0] || 'name';
    state.sortDir = kv[1] || 'asc';
    renderFiles();
  });

  $('#fileInput').addEventListener('change', function(){
    if (this.files.length) uploadFiles(this.files);
    this.value = '';
  });

  document.addEventListener('click', hideCtx);
  document.addEventListener('keydown', function(e){ if (e.key === 'Escape') hideCtx(); });

  $('#zoneD').addEventListener('contextmenu', function(e){
    if (e.target === $('#filesGrid') || e.target === $('#zoneD') || e.target === $('#emptyMsg') || e.target === $('#rubberBand')){
      e.preventDefault();
      showCtxMenu(e.clientX, e.clientY);
    }
  });

  window.addEventListener('hashchange', function(){ loadDir(currentPathFromHash()); });
  bindAlbum();
  bindSelection();
  bindKeyboard();
  bindDropUpload();
}

(async function init(){
  bind();
  applyThumbSize();
  setView('grid');
  await loadTree('/');
  loadDir(currentPathFromHash());
  if (!location.hash) history.replaceState(null, '', '#' + encodeURIComponent(state.path || '/'));
})();
</script>
</body>
</html>
)uSWAPP";

    std::string html = rawHTML;
    boost::replace_all(html, "__APP_TITLE__", escaped);
    return html;
}
