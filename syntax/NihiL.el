;; NihiL mode  -*- lexical-binding: t; -*-

(defconst NihiL-mode-syntax-table
  (with-syntax-table (copy-syntax-table)
    (modify-syntax-entry ?- ". 124b")
    (modify-syntax-entry ?* ". 23")
    (modify-syntax-entry ?\n "> b")
    (modify-syntax-entry ?' "\"") ; Single quote for character literals
    (modify-syntax-entry ?\" "\"") ; Double quote for string literals
    (modify-syntax-entry ?' ".")
    (syntax-table))
  "Syntax table for `NihiL-mode'.")

(defun NihiL-indent-line ()
  "Indent current line."
  (let (indent
        boi-p
        move-eol-p
        (point (point)))
    (save-excursion
      (back-to-indentation)
      (setq indent (car (syntax-ppss))
            boi-p (= point (point)))
      (when (and (eq (char-after) ?\n)
                 (not boi-p))
        (setq indent 0))
      (when boi-p
        (setq move-eol-p t))
      (when (or (eq (char-after) ?\))
                (eq (char-after) ?\}))
        (setq indent (1- indent)))
      (delete-region (line-beginning-position)
                     (point))
      (indent-to (* tab-width indent)))
    (when move-eol-p
      (move-end-of-line nil))))

(eval-and-compile
  (defconst NihiL-types
    '("void" "i8" "i16" "i32" "i64" "u8" "u16" "u32" "u64" "bool" "str" "size_t")
    "Type keywords for NihiL mode.")
  (defconst NihiL-keywords
    '("if" "else" "while" "let" "for" "proc" "return" "mut" "break" "macro" "exit"
      "extern" "enum" "struct" "import" "ref" "end" "export" "embed" "true" "false"
      "def" "in" "null" "type" "module" "where" "continue" "cast")
    "Non-type keywords for NihiL mode."))

(defconst NihiL-highlights
  `((,(concat "\\<" (regexp-opt NihiL-types) "\\>")
      . font-lock-type-face)
    (,(concat "\\<" (regexp-opt NihiL-keywords) "\\>")
      . font-lock-keyword-face)
    (,(concat "<" (regexp-opt NihiL-keywords) ">")
      . font-lock-keyword-face)
    (,(rx (group "\"" (zero-or-more (not (any "\"" "\\"))) (zero-or-one "\\\"") "\""))
      . font-lock-string-face) ; String literals
    (,(rx (group "'" (any "a-zA-Z0-9") "'"))
      . font-lock-string-face) ; Character literals
    (,(rx (group "--" (zero-or-more (not (any "\n"))))
          (group-n 1 (zero-or-more (any "\n"))))
     (1 font-lock-comment-delimiter-face)
     (2 font-lock-comment-face nil t))))

;;;###autoload
(define-derived-mode NihiL-mode prog-mode "NihiL"
  "Major Mode for editing NihiL source code."
  :syntax-table NihiL-mode-syntax-table
  (setq font-lock-defaults '(NihiL-highlights))
  (setq-local comment-start "--")
  (setq-local indent-tabs-mode nil)
  (setq-local tab-width 8)
  (setq-local indent-line-function #'NihiL-indent-line)
  (setq-local standard-indent 2))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.nl\\'" . NihiL-mode))

(provide 'NihiL-mode)
