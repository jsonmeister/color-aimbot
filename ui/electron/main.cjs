const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');

// Assuming the exe is in root/build/Release, config is in root.
// During dev: ui/electron/main.js runs, config is in ../../config.json?
// Let's settle on: config.json is in the CWD of the process.
// Backend (Release) is in root/build/Release
// UI (during dev) runs in root/colorbot/ui
const CONFIG_PATH = path.resolve(__dirname, '../../build/Release/config.json');

function createWindow() {
    const win = new BrowserWindow({
        width: 1000,
        height: 720,
        frame: false,
        transparent: true,
        resizable: false,
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.cjs'),
            backgroundThrottling: false,
            webSecurity: false
        },
        backgroundColor: '#00000000'
    });

    // Simple dev/prod check
    if (!app.isPackaged) {
        win.loadURL('http://localhost:5173');
        win.webContents.openDevTools({ mode: 'detach' });
    } else {
        win.loadFile(path.join(__dirname, '../dist/index.html'));
    }

    // Handle Window Controls
    ipcMain.on('window-close', () => win.close());
    ipcMain.on('window-minimize', () => win.minimize());

    // Config Handlers
    ipcMain.handle('read-config', async () => {
        try {
            if (fs.existsSync(CONFIG_PATH)) {
                const data = fs.readFileSync(CONFIG_PATH, 'utf-8');
                return JSON.parse(data);
            }
            return {};
        } catch (e) {
            console.error('[Electron] Config read error:', e);
            return {};
        }
    });

    ipcMain.on('save-config', (event, config) => {
        try {
            fs.writeFileSync(CONFIG_PATH, JSON.stringify(config, null, 4));
        } catch (e) {
            console.error('Failed to save config', e);
        }
    });
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});
