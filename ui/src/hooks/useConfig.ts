import { useState, useEffect, useCallback } from 'react';

// Define Config Types clearly for TS
export interface ProfileConfig {
    xSpeed: number;
    ySpeed: number;
    xFov: number;
    yFov: number;
    targetOffset: number;
    deadzone: number;
    humanize: boolean;
    patternVariation: number;
    rcs: boolean;
    always_active: boolean;
}

export interface VisualsConfig {
    enabled: boolean;
    draw_fov: boolean;
    draw_target: boolean;
}

export interface AppConfig {
    profile: string;
    debugMode: boolean;
    legit: ProfileConfig;
    semi: ProfileConfig;
    rage: ProfileConfig;
    visuals: VisualsConfig;
    [key: string]: any; // Index signature for dynamic access
}

const DEFAULT_PROFILE: ProfileConfig = {
    xSpeed: 0.15, ySpeed: 0.15, xFov: 60, yFov: 45,
    targetOffset: 8.0, deadzone: 4, humanize: true,
    patternVariation: 2, rcs: true, always_active: false
};

const DEFAULT_CONFIG: AppConfig = {
    profile: 'legit',
    debugMode: false,
    legit: { ...DEFAULT_PROFILE },
    semi: { ...DEFAULT_PROFILE, xSpeed: 0.4, ySpeed: 0.4, xFov: 100, yFov: 80, deadzone: 2, patternVariation: 1, rcs: false },
    rage: { ...DEFAULT_PROFILE, xSpeed: 0.8, ySpeed: 0.8, xFov: 200, yFov: 200, deadzone: 0, humanize: false, patternVariation: 0, always_active: true, rcs: false },
    visuals: { enabled: true, draw_fov: true, draw_target: true }
};

export function useConfig() {
    const [config, setConfig] = useState<AppConfig>(DEFAULT_CONFIG);
    const [isLoaded, setIsLoaded] = useState(false);

    useEffect(() => {
        if (window.electron) {
            window.electron.readConfig().then((data) => {
                if (data && Object.keys(data).length > 0) {
                    setConfig((prev) => ({ ...prev, ...data }));
                }
                setIsLoaded(true);
            });
        } else {
            setIsLoaded(true);
        }
    }, []);

    const updateConfig = useCallback((path: string[], value: any) => {
        setConfig((prev) => {
            const newConfig = { ...prev };
            let current: any = newConfig;
            for (let i = 0; i < path.length - 1; i++) {
                if (!current[path[i]]) current[path[i]] = {};
                current = current[path[i]];
            }
            current[path[path.length - 1]] = value;

            if (window.electron) {
                window.electron.saveConfig(newConfig);
            }
            return newConfig;
        });
    }, []);

    const setProfile = useCallback((profileName: string) => {
        updateConfig(['profile'], profileName);
    }, [updateConfig]);

    return { config, updateConfig, setProfile, isLoaded };
}
