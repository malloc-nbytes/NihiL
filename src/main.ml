let () =
  let path = "1.in" in
  let src = Io.load_file path in
  let _ = Lexer.lex path src in
  ()
