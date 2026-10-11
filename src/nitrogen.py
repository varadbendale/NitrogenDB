import tkinter as tk
from tkinter import font as tkfont

# ── theme ─────────────────────────────────────────────────────────────────────

BG         = "#000000"
FG         = "#ffffff"
DIM        = "#555555"
FONT_FAMILY = "Courier"
FONT_SIZE   = 13
PROMPT      = "numes> "


# ── mock query engine ─────────────────────────────────────────────────────────

def execute_query(filename, query):
    q = query.strip().lower()
    if q.startswith("select"):
        return (
            "+----+-----------+-----+\n"
            "| id | name      | age |\n"
            "+----+-----------+-----+\n"
            "| 1  | Aryan     | 21  |\n"
            "| 2  | Ravi      | 24  |\n"
            "| 3  | Sneha     | 22  |\n"
            "+----+-----------+-----+\n"
            "(3 rows)"
        )
    elif q.startswith("insert"):
        return "1 row affected"
    elif q.startswith("delete"):
        return "1 row affected"
    elif q.startswith("update"):
        return "1 row affected"
    else:
        return f"unrecognized query — type \\help for syntax"


# ── main app ──────────────────────────────────────────────────────────────────

class NumesApp(tk.Tk):

    def __init__(self):
        super().__init__()

        self.title("Numes")
        self.geometry("860x560")
        self.minsize(600, 400)
        self.configure(bg=BG)

        # one font object used everywhere
        self.font = tkfont.Font(family=FONT_FAMILY, size=FONT_SIZE)

        self.history       = []
        self.history_index = -1
        self.authenticated = False
        self.active_file   = ""

        self._build_ui()
        self._print_banner()
        self.file_entry.focus_set()


    # ── ui layout ─────────────────────────────────────────────────────────────

    def _build_ui(self):
        f = self.font

        # ── top bar ───────────────────────────────────────────────────────────
        bar = tk.Frame(self, bg=BG, height=36)
        bar.pack(fill="x", side="top")
        bar.pack_propagate(False)

        tk.Label(bar, text="file:", font=f, bg=BG, fg=FG).pack(
            side="left", padx=(14, 2), pady=6)

        self.file_entry = tk.Entry(
            bar, font=f, width=18,
            bg=BG, fg=FG, insertbackground=FG,
            relief="flat", highlightthickness=0, bd=0
        )
        self.file_entry.pack(side="left", pady=6)

        tk.Label(bar, text="pass:", font=f, bg=BG, fg=FG).pack(
            side="left", padx=(14, 2), pady=6)

        self.pass_entry = tk.Entry(
            bar, font=f, width=14, show="•",
            bg=BG, fg=FG, insertbackground=FG,
            relief="flat", highlightthickness=0, bd=0
        )
        self.pass_entry.pack(side="left", pady=6)

        self.connect_btn = tk.Label(
            bar, text="connect", font=f,
            bg=BG, fg=FG, cursor="hand2"
        )
        self.connect_btn.pack(side="left", padx=10, pady=6)
        self.connect_btn.bind("<Button-1>", lambda e: self._handle_connect())

        self.status_label = tk.Label(
            bar, text="● disconnected", font=f, bg=BG, fg=FG
        )
        self.status_label.pack(side="right", padx=14)

        # ── terminal ──────────────────────────────────────────────────────────
        self.terminal = tk.Text(
            self,
            bg=BG, fg=FG,
            font=f,
            insertbackground=FG,
            relief="flat", bd=0,
            highlightthickness=0,
            padx=16, pady=10,
            wrap="word",
            cursor="xterm",
            undo=False,
        )
        self.terminal.pack(fill="both", expand=True)

        self.terminal.tag_config("dim",    foreground=DIM)
        self.terminal.tag_config("normal", foreground=FG)

        self.terminal.bind("<Return>",          self._handle_enter)
        self.terminal.bind("<Up>",              self._history_up)
        self.terminal.bind("<Down>",            self._history_down)
        self.terminal.bind("<BackSpace>",       self._handle_backspace)
        self.terminal.bind("<Key>",             self._handle_key)
        self.terminal.bind("<Control-c>",       self._handle_ctrl_c)
        self.terminal.bind("<ButtonRelease-1>", self._clamp_cursor)
        self.terminal.bind("<ButtonRelease-2>", self._clamp_cursor)
        self.terminal.bind("<ButtonRelease-3>", self._clamp_cursor)

        self.file_entry.bind("<Return>", lambda e: self.pass_entry.focus_set())
        self.pass_entry.bind("<Return>", lambda e: self._handle_connect())


    # ── banner ────────────────────────────────────────────────────────────────

    def _print_banner(self):
        self._write(
            " NumesDB  v0.0.1\n",
            "dim"
        )
        self._new_prompt()


    # ── connect ───────────────────────────────────────────────────────────────

    def _handle_connect(self):
        filename = self.file_entry.get().strip()
        password = self.pass_entry.get().strip()

        if not filename:
            self._write("error: filename is required\n\n")
            self.file_entry.focus_set()
            return
        if not password:
            self._write("error: password is required\n\n")
            self.pass_entry.focus_set()
            return

        self.authenticated = True
        self.active_file   = filename
        self.status_label.configure(text=f"● {filename}")
        self._write(f"connected to {filename}\n\n")
        self.terminal.focus_set()


    # ── prompt ────────────────────────────────────────────────────────────────

    def _new_prompt(self):
        self.terminal.insert("end", PROMPT, "normal")
        self.terminal.see("end")
        self.terminal.mark_set("input_start", "end-1c")
        self.terminal.mark_gravity("input_start", "left")
        self.terminal.focus_set()

    def _get_input(self):
        return self.terminal.get("input_start", "end-1c")

    def _cursor_in_input(self):
        return self.terminal.compare("insert", ">=", "input_start")


    # ── key handlers ─────────────────────────────────────────────────────────

    def _handle_key(self, event):
        if event.keysym in ("Return", "BackSpace", "Up", "Down"):
            return
        if event.char and event.char.isprintable():
            if not self._cursor_in_input():
                self.terminal.mark_set("insert", "end")
            return
        return "break"

    def _handle_backspace(self, event=None):
        if not self._cursor_in_input():
            return "break"
        if self.terminal.compare("insert", "<=", "input_start"):
            return "break"

    def _handle_ctrl_c(self, event=None):
        self.terminal.insert("end", "^C\n")
        self._new_prompt()
        return "break"

    def _clamp_cursor(self, event=None):
        self.terminal.after(1, lambda: (
            self.terminal.mark_set("insert", "end")
            if not self._cursor_in_input() else None
        ))

    def _handle_enter(self, event=None):
        raw = self._get_input().strip()
        self.terminal.insert("end", "\n")

        if not raw:
            self._new_prompt()
            return "break"

        self.history.append(raw)
        self.history_index = -1

        if raw.startswith("\\"):
            self._handle_meta(raw)
        elif not self.authenticated:
            self._write("error: not connected — enter filename and password above\n\n")
        else:
            result = execute_query(self.active_file, raw)
            self._write(result + "\n\n")

        self._new_prompt()
        return "break"


    # ── meta commands ─────────────────────────────────────────────────────────

    def _handle_meta(self, cmd):
        if cmd in ("\\q", "\\quit"):
            self.quit()

        elif cmd == "\\help":
            self._write(
                "\n"
                "  commands\n"
                "  \\help          show this\n"
                "  \\q  \\quit      exit numes\n"
                "  \\clear         clear the screen\n"
                "  \\nodes         list mesh nodes\n"
                "  \\status        show connection info\n"
                "\n"
                "  query syntax\n"
                "  SELECT col FROM table WHERE col = val\n"
                "  INSERT INTO table (col) VALUES (val)\n"
                "  DELETE FROM table WHERE col = val\n"
                "  UPDATE table SET col = val WHERE id = 1\n"
                "\n",
                "dim"
            )

        elif cmd == "\\clear":
            self.terminal.delete("1.0", "end")

        elif cmd == "\\nodes":
            self._write(
                "\n"
                "  name          ip                status\n"
                "  ------------  ----------------  -----------\n"
                "  node1         192.168.1.10      alive\n"
                "  node2         192.168.1.11      alive\n"
                "  node3         192.168.1.12      unreachable\n"
                "\n"
                "  (3 nodes)\n\n"
            )

        elif cmd == "\\status":
            if self.authenticated:
                self._write(
                    f"\n"
                    f"  file     : {self.active_file}\n"
                    f"  node     : node1\n"
                    f"  ip       : 192.168.1.10\n"
                    f"  peers    : 2 connected\n\n"
                )
            else:
                self._write("not connected\n\n", "dim")

        else:
            self._write(f"unknown command '{cmd}' — type \\help\n\n")


    # ── history ───────────────────────────────────────────────────────────────

    def _history_up(self, event=None):
        if not self.history:
            return "break"
        if self.history_index == -1:
            self.history_index = len(self.history) - 1
        elif self.history_index > 0:
            self.history_index -= 1
        self._set_input(self.history[self.history_index])
        return "break"

    def _history_down(self, event=None):
        if self.history_index == -1:
            return "break"
        self.history_index += 1
        if self.history_index >= len(self.history):
            self.history_index = -1
            self._set_input("")
        else:
            self._set_input(self.history[self.history_index])
        return "break"

    def _set_input(self, text):
        self.terminal.delete("input_start", "end-1c")
        self.terminal.insert("input_start", text)
        self.terminal.mark_set("insert", "end")


    # ── write ─────────────────────────────────────────────────────────────────

    def _write(self, text, tag="normal"):
        self.terminal.insert("end", text, tag)
        self.terminal.see("end")


# ── run ───────────────────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = NumesApp()
    app.mainloop()