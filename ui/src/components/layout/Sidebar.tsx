import { motion } from 'framer-motion';
import { Shield, type LucideIcon } from 'lucide-react';
import { cn } from '@/lib/utils';

interface Tab {
    id: string;
    label: string;
    icon: LucideIcon;
}

interface SidebarProps {
    tabs: Tab[];
    activeTab: string;
    onTabChange: (id: string) => void;
    status?: string;
}

export function Sidebar({ tabs, activeTab, onTabChange, status = "UNDETECTED" }: SidebarProps) {
    return (
        <div className="w-64 h-full flex flex-col bg-black/85 backdrop-blur-xl border-r border-white/5">
            <div className="p-6 pt-10">
                <h1 className="text-xl font-bold text-white tracking-wider flex items-center gap-2 uppercase">
                    Json's Colorbot
                </h1>
                <p className="text-xs text-zinc-500 mt-1 uppercase tracking-widest font-semibold flex items-center gap-1">
                    <Shield size={10} /> Private Build
                </p>
            </div>

            <nav className="flex-1 px-4 space-y-2 mt-4">
                {tabs.map((tab) => {
                    const Icon = tab.icon;
                    const isActive = activeTab === tab.id;
                    return (
                        <button
                            key={tab.id}
                            onClick={() => onTabChange(tab.id)}
                            className={cn(
                                "w-full flex items-center gap-3 px-3 py-2 rounded-lg transition-all duration-200 group relative overflow-hidden text-sm",
                                isActive ? "bg-white/5 text-white shadow-lg shadow-purple-900/10 border border-white/5" : "text-zinc-400 hover:text-white hover:bg-white/5"
                            )}
                        >
                            {isActive && (
                                <motion.div
                                    layoutId="activeTab"
                                    className="absolute inset-0 bg-gradient-to-r from-purple-500/10 to-transparent pointer-events-none"
                                    initial={false}
                                    transition={{ type: "spring", stiffness: 500, damping: 30 }}
                                />
                            )}
                            <Icon size={20} className={cn("relative z-10", isActive ? "text-purple-400" : "group-hover:text-purple-300")} />
                            <span className="relative z-10 font-medium">{tab.label}</span>
                        </button>
                    );
                })}
            </nav>

            <div className="p-4 border-t border-white/5 bg-black/20">
                <div className="flex items-center gap-3 px-4 py-2 rounded-lg bg-green-500/5 border border-green-500/10">
                    <div className="w-2 h-2 rounded-full bg-green-500 animate-pulse shadow-[0_0_8px_rgba(34,197,94,0.6)]" />
                    <span className="text-xs font-semibold text-green-400 tracking-wide">{status}</span>
                </div>
            </div>
        </div>
    );
}
