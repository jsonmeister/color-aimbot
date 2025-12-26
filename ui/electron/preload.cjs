const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electron', {
    close: () => ipcRenderer.send('window-close'),
    minimize: () => ipcRenderer.send('window-minimize'),
    readConfig: () => ipcRenderer.invoke('read-config'),
    saveConfig: (config) => ipcRenderer.send('save-config', config)
});
