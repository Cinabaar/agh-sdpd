((nil . ((eval . (progn
                   (require 'projectile)
                   (puthash (projectile-project-root)
                           (format "./compile.sh %s" (projectile-project-root)) 
                            projectile-compilation-cmd-map))))))

