((nil
  . ((eval . (progn
               (defvar aoc2025/dir-locals-deps-ok nil)
               (defvar aoc2025/dir-locals-missing-deps-reported nil)

               (unless aoc2025/dir-locals-deps-ok
                 (let ((missing '()))
                   (dolist (pkg '(transient cl-lib subr-x))
                     (unless (require pkg nil 'noerror)
                       (push pkg missing)))
                   (if missing
                       (unless aoc2025/dir-locals-missing-deps-reported
                         (setq aoc2025/dir-locals-missing-deps-reported t)
                         (message "aoc2025/.dir-locals: missing packages: %s (C-c b disabled)"
                                  (mapconcat #'symbol-name (nreverse missing) ", ")))
                     (setq aoc2025/dir-locals-deps-ok t))))

               (when aoc2025/dir-locals-deps-ok
		 (let* ((root (or (locate-dominating-file default-directory ".dir-locals.el") default-directory))
			(base-compile-cmd (if (eq system-type 'windows-nt)
					      (format "cd %s && bs.exe" (shell-quote-argument (expand-file-name root)))
					    (format "cd %s && ./bs" (shell-quote-argument (expand-file-name root))))))

		   (defun aoc2025/build-flag-on-p (flag)
		     (member flag (transient-get-value)))

		   (defun aoc2025/build-flag-label (label flag)
		     (propertize label
				 'face (if (aoc2025/build-flag-on-p flag)
					   'success
					 'default)))

		   (defun aoc2025/build-dedupe-flags (args)
		     (cl-remove-duplicates args :test #'string= :from-end t))

		   (defun aoc2025/build-final-args (args)
		     (thread-last args
				  (aoc2025/build-dedupe-flags)))


		   (transient-define-argument aoc2025/toggle-debug ()
		     "Toggle debug flag."
		     :class 'transient-switch
		     :key "-d"
		     :argument "--debug"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Enable debug symbols and disable optimizations" "--debug")))

		   (transient-define-argument aoc2025/toggle-verbose ()
		     "Toggle verbose flag."
		     :class 'transient-switch
		     :key "-v"
		     :argument "--verbose"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Enable verbose output" "--verbose")))

		   (transient-define-argument aoc2025/toggle-asan ()
		     "Toggle asan flag."
		     :class 'transient-switch
		     :key "-a"
		     :argument "--asan"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Use address sanitizer" "--asan")))

		   (transient-define-argument aoc2025/toggle-ubsan ()
		     "Toggle ubsan flag."
		     :class 'transient-switch
		     :key "-u"
		     :argument "--ubsan"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Use undefined behavior sanitizer" "--ubsan"))
		     :if (lambda () (not (eq system-type 'windows-nt))))

		   (transient-define-argument aoc2025/set-compiler ()
		     "Set compiler."
		     :class 'transient-option
		     :key "--compiler"
		     :argument "--compiler="
		     :choices (lambda () (if (eq system-type 'windows-nt) (list "cl" "clang-cl") (list "gcc" "clang")))
		     :description
		     (lambda (obj)
		       (let ((val (oref obj value)))
			 "Specify compiler")))

		   (transient-define-argument aoc2025/toggle-no-cache ()
		     "Toggle no-cache flag."
		     :class 'transient-switch
		     :key "--no-cache"
		     :argument "--no-cache"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Disable build caching" "--no-cache")))

		   (transient-define-argument aoc2025/toggle-emit-compile-commands ()
		     "Toggle emit compile commands flag."
		     :class 'transient-switch
		     :key "--clangd"
		     :argument "--emit-compile-commands"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Generate compile_commands.json file for clangd" "--emit-compile-commands")))

		   (transient-define-argument aoc2025/toggle-emit-vscode-tasks ()
		     "Toggle emit compile commands flag."
		     :class 'transient-switch
		     :key "--vscode"
		     :argument "--emit-vscode-tasks"
		     :description (lambda (_obj)
				    (aoc2025/build-flag-label "Generate build and run tasks for VSCode" "--emit-vscode-tasks")))

		   (transient-define-argument aoc2025/specify-jobs ()
		     "Set number of jobs (integer). Empty = auto."
		     :class 'transient-option
		     :shortarg "-j"
		     :argument "--jobs="
		     :description "Specify max amount of concurrent jobs"
		     :reader
		     (lambda (&rest _ignore)
		       (let ((input (read-string "Jobs (>0, empty = auto): ")))
			 (cond
			  ((string-empty-p input) nil)
			  ((> (string-to-number input) 0)
			   input)
			  (t
			   (user-error "Please enter an integer > 0 or leave empty for auto"))))))

		   (transient-define-suffix aoc2025/build (args)
		     "Build Aoc2025."
		     :key "b"
		     :description "Build"
		     (interactive (list (transient-args 'aoc2025/transient)))
		     (let* ((final-args (aoc2025/build-final-args args))
			    (mode       (transient-arg-value "--compiler=" final-args))
			    (cmd-list   (cons base-compile-cmd final-args))
			    (cmd-str    (string-join cmd-list " ")))
		       (compile cmd-str)))

		   (transient-define-suffix aoc2025/build-and-run (args)
		     "Build and run Aoc2025."
		     :key "r"
		     :description "Build and run"
		     (interactive (list (transient-args 'aoc2025/transient)))
		     (aoc2025/build (append args '("--run"))))

		   (transient-define-suffix aoc2025/build-and-package (args)
		     "Build and package distribution of Aoc2025."
		     :key "p"
		     :description "Build and package distribution"
		     (interactive (list (transient-args 'aoc2025/transient)))
		     (aoc2025/build (append args '("--package"))))

		   (transient-define-suffix aoc2025/clean (args)
		     "Clean all build artifacts."
		     :key "c"
		     :description "Clean all build artifacts"
		     (interactive (list (transient-args 'aoc2025/transient)))
		     (aoc2025/build (append args '("--clean"))))

		   (transient-define-suffix aoc2025/help (args)
		     "Show bs CLI help menu."
		     :key "h"
		     :description "Show the CLI help menu of bs"
		     (interactive (list (transient-args 'aoc2025/transient)))
		     (aoc2025/build (append args '("--help"))))

		   (transient-define-prefix aoc2025/transient ()
		     "Build menu for Aoc2025"
		     ["Options"
		      (aoc2025/toggle-debug)
		      (aoc2025/toggle-verbose)
		      (aoc2025/specify-jobs)
		      (aoc2025/toggle-asan)
		      (aoc2025/toggle-ubsan)
		      (aoc2025/set-compiler)
		      (aoc2025/toggle-no-cache)
		      (aoc2025/toggle-emit-compile-commands)
		      (aoc2025/toggle-emit-vscode-tasks)]
		     ["Actions"
		      (aoc2025/build)
		      (aoc2025/build-and-run)
		      (aoc2025/build-and-package)
		      (aoc2025/clean)
		      (aoc2025/help)]
		     [("q" "Quit" transient-quit-one)])

		   (local-set-key (kbd "C-c b") 'aoc2025/transient))))))))
