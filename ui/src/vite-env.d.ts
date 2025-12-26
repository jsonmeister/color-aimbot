export { };

declare global {
    interface Window {
        electron?: {
            close: () => void;
            minimize: () => void;
            readConfig: () => Promise<any>;
            saveConfig: (config: any) => void;
        };
    }
}
