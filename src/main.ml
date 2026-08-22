let () =
  let path = "1.in" in
  let src = Io.load_file path in
  let l = Lexer.from_file path src in
  Lexer.dump l;
  let _ = Parser.parse l in
  ()
