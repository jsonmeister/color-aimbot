import { useRef } from 'react';
import { cn } from '@/lib/utils';

interface SliderProps {
    label: string;
    value: number;
    min: number;
    max: number;
    step?: number;
    onChange: (val: number) => void;
    formatValue?: (val: number) => string;
}

export function Slider({ label, value, min, max, step = 1, onChange, formatValue }: SliderProps) {
    const ref = useRef<HTMLInputElement>(null);
    const percentage = ((value - min) / (max - min)) * 100;

    return (
        <div className="space-y-3">
            <div className="flex justify-between items-center text-sm font-medium">
                <span className="text-zinc-400">{label}</span>
                <span className="text-white font-mono bg-white/5 px-2 py-0.5 rounded text-xs border border-white/5">
                    {formatValue ? formatValue(value) : value}
                </span>
            </div>
            <div className="relative h-6 flex items-center group">
                {/* Track Background */}
                <div className="absolute w-full h-1.5 bg-zinc-800 rounded-full overflow-hidden border border-white/5">
                    {/* Fill */}
                    <div
                        className="h-full bg-gradient-to-r from-purple-600 to-pink-500 transition-all duration-75 ease-out"
                        style={{ width: `${percentage}%` }}
                    />
                </div>

                {/* Thumb (Visual Only due to input overlay) */}
                <div
                    className="absolute w-4 h-4 bg-white rounded-full shadow-lg shadow-black/50 pointer-events-none transition-all duration-75 ease-out group-hover:scale-110 border-2 border-purple-500"
                    style={{ left: `calc(${percentage}% - 8px)` }}
                />

                <input
                    type="range"
                    min={min}
                    max={max}
                    step={step}
                    value={value}
                    onChange={(e) => onChange(Number(e.target.value))}
                    className="absolute inset-0 w-full h-full opacity-0 cursor-pointer"
                />
            </div>
        </div>
    );
}
