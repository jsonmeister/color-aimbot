import { useState } from 'react';
import { motion, AnimatePresence } from 'framer-motion';
import { Crosshair, Zap, Shield } from 'lucide-react';

import { Sidebar } from '@/components/layout/Sidebar';
import { ProfileSettings } from '@/components/features/ProfileSettings';
import { useConfig } from '@/hooks/useConfig';

const TABS = [
  { id: 'legit', label: 'Legit', icon: Shield },
  { id: 'semi', label: 'Semi', icon: Crosshair },
  { id: 'rage', label: 'Rage', icon: Zap },
];

function App() {
  const [activeTab, setActiveTab] = useState('legit');
  const { config, updateConfig, setProfile } = useConfig();

  const handleMinimize = () => window.electron?.minimize();
  const handleClose = () => window.electron?.close();

  const handleProfileUpdate = (key: any, value: any) => {
    updateConfig([activeTab, key], value);
  };

  const activeProfileConfig = config[activeTab] || config.legit;

  return (
    <div className="flex h-screen w-full bg-transparent text-white font-sans selection:bg-purple-500/30 overflow-hidden">
      <Sidebar
        tabs={TABS}
        activeTab={activeTab}
        onTabChange={setActiveTab}
      />

      {/* Main Content */}
      <div className="flex-1 h-full bg-zinc-900/90 flex flex-col relative">
        <div className="absolute inset-0 bg-[url('https://grainy-gradients.vercel.app/noise.svg')] opacity-20 pointer-events-none mix-blend-overlay" />

        {/* Top Bar */}
        <div className="h-10 w-full drag-region flex items-center justify-end px-3 space-x-2 z-50">
          <button onClick={handleMinimize} className="p-2 hover:bg-white/10 rounded-md text-zinc-400 hover:text-white transition-colors no-drag">
            <div className="w-3 h-0.5 bg-current rounded-full" />
          </button>
          <button onClick={handleClose} className="p-2 hover:bg-red-500/20 rounded-md text-zinc-400 hover:text-red-400 transition-colors no-drag">
            <div className="w-3 h-3 relative">
              <div className="absolute inset-0 flex items-center justify-center transform rotate-45 bg-current h-0.5 top-1.5" />
              <div className="absolute inset-0 flex items-center justify-center transform -rotate-45 bg-current h-0.5 top-1.5" />
            </div>
          </button>
        </div>

        <div className="flex-1 px-8 pb-8 overflow-y-auto no-scrollbar relative z-10">
          <header className="mb-8">
            <h2 className="text-3xl font-bold text-white capitalize flex items-center gap-3">
              {activeTab} <span className="text-zinc-600 text-lg font-normal">/ Configuration</span>
            </h2>
          </header>

          <AnimatePresence mode='wait'>
            <motion.div
              key={activeTab}
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              exit={{ opacity: 0, x: -20 }}
              transition={{ duration: 0.2 }}
            >
              <ProfileSettings
                config={activeProfileConfig}
                isActive={config.profile === activeTab}
                onUpdate={handleProfileUpdate}
                onActivate={() => setProfile(activeTab)}
              />
            </motion.div>
          </AnimatePresence>
        </div>
      </div>
    </div>
  );
}

export default App;
