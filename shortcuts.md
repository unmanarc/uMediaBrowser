# Keyboard Shortcuts Reference

Reference for the keyboard shortcuts available in the **uMediaBrowser** web
file manager UI (PC / desktop mode). All shortcuts are client-side only (implemented
in the HTML/JS served by the server — see `src/html_index_app.h`) and work when the
browser viewport is in desktop layout (not the phone layout).

## Behavior & Scope

- **PC mode only** — the handlers return early when the phone media query is active,
  so nothing interferes with touch interaction on small screens.
- **Ignored while typing** — shortcuts do not fire when focus is in an `<input>`,
  `<select>`, or `<textarea>` (e.g. the location bar or the filter box).
- **Album overlay** — when the full-screen album is open, it owns the keyboard
  (`←`, `→`, `Esc`); the file-manager shortcuts are suspended.
- **Keyboard cursor** — arrow keys move a highlighted cursor (`.item.active`) over the
  items in their *rendered order* (filter + sort applied). The cursor is kept in sync
  with mouse clicks: after any mouse selection the cursor jumps to the last selected item.

## Navigation

| Key | Action |
|-----|--------|
| `↑` / `↓` / `←` / `→` | Move the keyboard cursor (in grid view, left/right also move the cursor) |
| `Home` | Jump cursor to the first item (and select it) |
| `End` | Jump cursor to the last item (and select it) |
| `Enter` | Open the active item (enter folder / open or download file) |
| `Backspace` | **With a selection:** delete the selection (after confirm). **Without selection:** navigate to the parent directory |
| `Esc` | Close the context menu and clear the selection |

## Selection

| Key | Action |
|-----|--------|
| `Space` | Toggle selection of the active item |
| `Ctrl + A` | Select all currently visible items |
| `Shift + ↑/↓/←/→` | Extend the selection as a range from the last selected item |
| `Ctrl + ↑/↓/←/→` | Move the cursor and toggle the item in/out of the selection |

(Plain arrow movement without modifiers selects only the active item, mirroring a
single click.)

## Actions

| Key | Action |
|-----|--------|
| `Delete` | Delete the selected item(s) (after confirm; requires write permission) |
| `Ctrl + D` | Download the selected item (single file) |
| `Ctrl + X` | Archive selection as `.zip` (when archiving is enabled) |
| `Ctrl + Shift + D` | Archive selection as `.tar.gz` (when archiving is enabled) |
| `r` | Refresh the current directory |
| `n` | Create a new folder in the current directory (prompts for a name) |
| `u` | Open the upload dialog for the current directory |
| `a` | Open the full-screen album for the current directory's media |

## View & Focus

| Key | Action |
|-----|--------|
| `g` | Switch to grid view |
| `l` | Switch to list view |
| `f` | Focus the filter box |
| `p` | Focus the location (path) bar and select its text |
| `Ctrl + L` | Same as `p` (focus + select the location bar) |
| `F` | Toggle fullscreen |

## Album overlay (fullscreen media viewer)

| Key | Action |
|-----|--------|
| `←` | Previous item |
| `→` | Next item |
| `Esc` | Close the album |
