import { cn } from '@/lib/utils';
import { motion } from 'framer-motion';

interface SwitchProps {
    label: string;
    checked: boolean;
    onChange: (checked: boolean) => void;
    description?: string;
}

export function Switch({ label, checked, onChange, description }: SwitchProps) {
    return (
        <div className="flex items-center justify-between p-3 rounded-xl bg-white/5 border border-white/5 hover:bg-white/10 transition-colors group cursor-pointer" onClick={() => onChange(!checked)}>
            <div className="pr-4">
                <div className="text-sm font-medium text-zinc-200 group-hover:text-white transition-colors">{label}</div>
                {description && <div className="text-xs text-zinc-500 mt-0.5">{description}</div>}
            </div>

            <div className={cn(
                "w-11 h-6 rounded-full relative transition-colors duration-300 ease-in-out border border-transparent",
                checked ? "bg-purple-600 border-purple-500/50 shadow-lg shadow-purple-900/20" : "bg-zinc-800 border-white/5"
            )}>
                <motion.div
                    className={cn("absolute top-0.5 left-0.5 w-5 h-5 bg-white rounded-full shadow-md")}
                    animate={{ x: checked ? 20 : 0 }}
                    transition={{ type: "spring", stiffness: 500, damping: 30 }}
                />
            </div>
        </div>
    );
}
