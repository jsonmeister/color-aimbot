import { Crosshair, Zap, CheckCircle } from 'lucide-react';
import { Slider } from '@/components/ui/Slider';
import { Switch } from '@/components/ui/Switch';
import type { ProfileConfig } from '@/hooks/useConfig';

interface ProfileSettingsProps {
    config: ProfileConfig;
    isActive: boolean;
    onUpdate: (key: keyof ProfileConfig, value: any) => void;
    onActivate: () => void;
}

export function ProfileSettings({ config, isActive, onUpdate, onActivate }: ProfileSettingsProps) {
    return (
        <div className="space-y-6">
            <div className="flex items-center justify-between bg-white/5 p-4 rounded-xl border border-white/5">
                <div>
                    <h4 className="font-semibold text-white">Active Status</h4>
                    <p className="text-zinc-500 text-xs">Is this profile currently active?</p>
                </div>
                {isActive ? (
                    <div className="flex items-center gap-2 text-green-400 bg-green-500/10 px-3 py-1.5 rounded-lg border border-green-500/20">
                        <CheckCircle size={16} />
                        <span className="text-sm font-bold">ACTIVE</span>
                    </div>
                ) : (
                    <button
                        onClick={onActivate}
                        className="px-3 py-1.5 text-sm bg-white/5 hover:bg-white/10 rounded-lg text-zinc-300 transition-colors border border-white/5"
                    >
                        Activate
                    </button>
                )}
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
                <div className="space-y-4 p-4 rounded-xl bg-white/5 border border-white/5">
                    <h3 className="text-purple-300 font-semibold flex items-center gap-2">
                        <Crosshair size={18} /> Aim Settings
                    </h3>

                    <Slider
                        label="Aim Speed (Horizontal)"
                        value={config.xSpeed} min={0.01} max={2.0} step={0.01}
                        onChange={(v) => onUpdate('xSpeed', v)}
                        formatValue={(v) => v.toFixed(2) + 'x'}
                    />
                    <Slider
                        label="Aim Speed (Vertical)"
                        value={config.ySpeed} min={0.01} max={2.0} step={0.01}
                        onChange={(v) => onUpdate('ySpeed', v)}
                        formatValue={(v) => v.toFixed(2) + 'y'}
                    />

                    <div className="grid grid-cols-2 gap-4">
                        <Slider
                            label="FOV X" value={config.xFov} min={10} max={800}
                            onChange={(v) => onUpdate('xFov', v)}
                            formatValue={(v) => v + 'px'}
                        />
                        <Slider
                            label="FOV Y" value={config.yFov} min={10} max={600}
                            onChange={(v) => onUpdate('yFov', v)}
                            formatValue={(v) => v + 'px'}
                        />
                    </div>
                </div>

                <div className="space-y-4 p-4 rounded-xl bg-white/5 border border-white/5">
                    <h3 className="text-pink-300 font-semibold flex items-center gap-2">
                        <Zap size={18} /> Behavior
                    </h3>
                    <Slider
                        label="Smooth / Deadzone" value={config.deadzone} min={0} max={10}
                        onChange={(v) => onUpdate('deadzone', v)}
                        formatValue={(v) => v + 'px'}
                    />
                    <Slider
                        label="Target Offset (Head)" value={config.targetOffset} min={-20} max={20}
                        onChange={(v) => onUpdate('targetOffset', v)}
                        formatValue={(v) => v + 'px'}
                    />

                    <div className="space-y-3 pt-2">
                        <Switch
                            label="Humanization"
                            description="Add random noise"
                            checked={config.humanize}
                            onChange={(v) => onUpdate('humanize', v)}
                        />
                        {config.humanize && (
                            <Slider
                                label="Pattern Variation" value={config.patternVariation} min={0} max={5}
                                onChange={(v) => onUpdate('patternVariation', v)}
                            />
                        )}

                        <Switch
                            label="Always Active"
                            description="Lock on target without key"
                            checked={config.always_active}
                            onChange={(v) => onUpdate('always_active', v)}
                        />
                        <Switch
                            label="RCS (Recoil Control)"
                            description="Stand-alone recoil compensation"
                            checked={config.rcs}
                            onChange={(v) => onUpdate('rcs', v)}
                        />
                    </div>
                </div>
            </div>
        </div>
    );
}
