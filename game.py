import tkinter as tk
from tkinter import messagebox
import subprocess
import enum
import os
import sys
import stat

# --- CONSTANTES ---
COLORS = ["#F0D9B5", "#B58863", "#BBCB2B"]
BOARD_SIZE = 600
SQUARE_SIZE = BOARD_SIZE // 8

PIECE_IMAGES = {
    "P": "white-pawn.png", "R": "white-rook.png", "N": "white-knight.png",
    "B": "white-bishop.png", "Q": "white-queen.png", "K": "white-king.png",
    "p": "black-pawn.png", "r": "black-rook.png", "n": "black-knight.png",
    "b": "black-bishop.png", "q": "black-queen.png", "k": "black-king.png",
    "A": "white-princess.png", "a": "black-princess.png",
    "E": "white-empress.png", "e": "black-empress.png",
    "G": "white-grassh.png", "g": "black-grassh.png",
    "H": "white-nightrd.png", "h": "black-nightrd.png",
}

class Variante(enum.Enum):
    CLASSIC = 0
    FAIRY = 1

# --- MOTEUR ---
class Engine():
    def __init__(self, engine_path: str, variant: str, nb_ai: int = 0):
        if not os.path.exists(engine_path):
            raise FileNotFoundError(f"Moteur introuvable à : {engine_path}")
        
        try:
            st = os.stat(engine_path)
            os.chmod(engine_path, st.st_mode | stat.S_IEXEC)
        except Exception:
            pass
        
        mode_arg = "pvp" if nb_ai == 0 else "pve"
        
        cmd = [engine_path, variant, mode_arg] 
        
        try:
            self.process = subprocess.Popen(
                cmd,
                stdin=subprocess.PIPE, 
                stdout=subprocess.PIPE, 
                stderr=subprocess.PIPE, 
                text=True, 
                bufsize=1
            )
        except PermissionError:
            print(f"ERREUR : Permission refusée sur '{engine_path}'.")
            sys.exit(1)
        except OSError as e:
            print(f"ERREUR : Impossible de lancer l'exécutable ({e}).")
            sys.exit(1)

    def send_request(self, request: str | None):
        """Envoie une commande de JEU et attend 8 lignes."""
        if self.process.stdin:
            if request is not None:
                try:
                    self.process.stdin.write(request + "\n")
                    self.process.stdin.flush()
                except BrokenPipeError:
                    print("Erreur : Le moteur C++ s'est arrêté.")
                    return []
            return self.read_board()
        return []
    
    def send_query(self, query: str):
        """Envoie une requête (ex: 'possible e2') et attend 1 ligne."""
        if self.process.stdin:
            try:
                self.process.stdin.write(query + "\n")
                self.process.stdin.flush()
                line = self.process.stdout.readline()
                return line.strip()
            except BrokenPipeError:
                return ""
        return ""
            
    def close(self):
        if self.process:
            self.process.terminate()

    def read_board(self):
        lines = []
        while len(lines) < 8:
            line = self.process.stdout.readline()
            if not line: break
            stripped = line.strip()
            if not stripped: continue
            lines.append(stripped)
        return lines

# --- LOGIQUE UI ---
class Move():
    def __init__(self, canvas, game_instance):
        self.canvas = canvas
        self.game = game_instance 
        self.selected = None
        self.highlighted = None
        self.suggestion_rects = [] # Stocke les carrés jaunes
        self.canvas.bind("<Button-1>", self.click)

    def highlight(self, col, row):
        x0 = col * SQUARE_SIZE
        y0 = row * SQUARE_SIZE
        x1 = x0 + SQUARE_SIZE
        y1 = y0 + SQUARE_SIZE
        self.highlighted = self.canvas.create_rectangle(x0, y0, x1, y1, outline=COLORS[-1], width=4)

    def show_suggestions(self, moves_str):
        squares = moves_str.split()
        for sq in squares:
            if len(sq) < 2: continue
            col = ord(sq[0]) - ord('a')
            row = 8 - int(sq[1])
            
            x0 = col * SQUARE_SIZE
            y0 = row * SQUARE_SIZE
            x1 = x0 + SQUARE_SIZE
            y1 = y0 + SQUARE_SIZE
            
            rect_id = self.canvas.create_rectangle(x0+2, y0+2, x1-2, y1-2, outline="#FFD700", width=4, tags="suggestion")
            self.suggestion_rects.append(rect_id)

    def clear_suggestions(self):
        for rect_id in self.suggestion_rects:
            self.canvas.delete(rect_id)
        self.suggestion_rects.clear()

    def click(self, event):
        col, row = event.x // SQUARE_SIZE, event.y // SQUARE_SIZE
        pos = f"{chr(col + ord('a'))}{8 - row}"
        
        if self.selected is None:
            piece = self.game.board[pos]
            if not piece: return

            self.selected = pos
            self.highlight(col, row)
            
            response = self.game.engine.send_query(f"possible {pos}")
            if response:
                self.show_suggestions(response)
        
        else:
            self.clear_suggestions()

            if self.selected != pos:
                self.game.submit_move(self.selected, pos)
            
            self.selected = None
            if self.highlighted:
                self.canvas.delete(self.highlighted)
            self.highlighted = None

class Board():
    def __init__(self, var: Variante = Variante.CLASSIC):
        self.board = [[] for _ in range(8)]

    def __getitem__(self, pos: str):
        if len(pos) != 2: return None
        col = ord(pos[0]) - ord('a')
        row = 8 - int(pos[1])
        if not (0 <= col < 8 and 0 <= row < 8): return None
        if row >= len(self.board) or col >= len(self.board[row]): return None
        piece = self.board[row][col] 
        return None if piece == "-" else piece

    def update(self, board_output: list):
        if len(board_output) != 8:
            print(f"DEBUG: Reçu {len(board_output)} lignes au lieu de 8. Ignoré.")
            return 
        new_board = []
        for line in board_output:
            parts = line.split()
            if len(parts) != 8: return 
            new_board.append(parts)
        self.board = new_board

    def get_raw_data(self):
        return [row[:] for row in self.board]

class DisplayGame():
    def __init__(self, root, variant_str: str, engine_path: str, nb_ai: int = 0):
        self.root = root
        self.engine = Engine(engine_path, variant_str, nb_ai)
        self.board = Board(Variante.FAIRY if variant_str == "fairy" else Variante.CLASSIC)
        
        self.nb_ai = nb_ai
        self.gamemode = "PvP" if nb_ai == 0 else "PvAI" if nb_ai == 1 else "AIvAI"
        
        self.root.title(f"Chess : {self.gamemode}")
        self.canvas = tk.Canvas(root, width=BOARD_SIZE, height=BOARD_SIZE)
        self.canvas.pack()
        
        self.move_handler = Move(self.canvas, self)

        self.piece_images = {}
        asset_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
        for piece, filename in PIECE_IMAGES.items():
            path = os.path.join(asset_dir, filename)
            if os.path.exists(path):
                img = tk.PhotoImage(file=path)
                img = img.subsample(2, 2)
                self.piece_images[piece] = img

        self.act(self.engine.read_board())


    def draw_board(self):
        self.canvas.delete("all")
        for row in range(8):
            for col in range(8):
                x0, y0 = col * SQUARE_SIZE, row * SQUARE_SIZE
                x1, y1 = x0 + SQUARE_SIZE, y0 + SQUARE_SIZE
                
                self.canvas.create_rectangle(x0, y0, x1, y1, fill=COLORS[(row + col) % 2], outline="")
                
                pos = f"{chr(col + ord('a'))}{8 - row}"
                piece = self.board[pos]
                
                if piece in self.piece_images:
                    self.canvas.create_image(x0 + SQUARE_SIZE//2, y0 + SQUARE_SIZE//2, anchor=tk.CENTER, image=self.piece_images[piece])

    def submit_move(self, pos1, pos2):
        piece = self.board[pos1]
        if not piece: return

        is_promo = (piece == 'P' and pos2[1] == '8') or (piece == 'p' and pos2[1] == '1')
        cmd = f"{pos1} {pos2}" + (" q" if is_promo else "")
        print(f"Coup joueur: {cmd}")

        previous_state = self.board.get_raw_data()
        self.ask_engine(cmd)
        
        current_state = self.board.get_raw_data()
        if previous_state == current_state:
            print(">> Coup illégal détecté.")
            return

        if self.gamemode == "PvAI":
            self.root.update()
            self.ask_engine(None)

    def ask_engine(self, command=None):
        ans = self.engine.send_request(command)
        if ans: self.act(ans)

    def act(self, answer: list):
        self.board.update(answer)
        self.draw_board()

# --- RECHERCHE ET LANCEMENT ---
def find_executable(start_dir, exe_name):
    print(f"Scanning dans : {start_dir} ...")
    for root, dirs, files in os.walk(start_dir):
        if exe_name in files:
            full_path = os.path.join(root, exe_name)
            if os.path.isfile(full_path):
                return full_path
    return None

def ask_settings(root):
    """Affiche une fenêtre pour configurer la Variante ET le Mode de jeu"""
    
    var_variant = tk.StringVar(value="classic")
    var_mode = tk.StringVar(value="pvp")
    
    dialog = tk.Toplevel(root)
    dialog.title("Configuration de la partie")
    dialog.geometry("350x250")
    
    tk.Label(dialog, text="1. Choisissez la variante :", font=("Arial", 11, "bold")).pack(pady=(15, 5))
    
    frame_var = tk.Frame(dialog)
    frame_var.pack()
    tk.Radiobutton(frame_var, text="Classique", variable=var_variant, value="classic").pack(side=tk.LEFT, padx=10)
    tk.Radiobutton(frame_var, text="Féerique (Fairy)", variable=var_variant, value="fairy").pack(side=tk.LEFT, padx=10)
    
    tk.Label(dialog, text="2. Choisissez le mode :", font=("Arial", 11, "bold")).pack(pady=(20, 5))
    
    frame_mode = tk.Frame(dialog)
    frame_mode.pack()
    tk.Radiobutton(frame_mode, text="Joueur vs Joueur (PvP)", variable=var_mode, value="pvp").pack(side=tk.LEFT, padx=10)
    tk.Radiobutton(frame_mode, text="Joueur vs IA (PvE)", variable=var_mode, value="pve").pack(side=tk.LEFT, padx=10)
    
    def on_submit():
        dialog.destroy()
        
    tk.Button(dialog, text="LANCER LA PARTIE", command=on_submit, 
              bg="#BBCB2B", font=("Arial", 10, "bold"), width=20, height=2).pack(pady=20)
    
    root.wait_window(dialog)
    
    return var_variant.get(), var_mode.get()

if __name__== "__main__":
    root = tk.Tk()
    root.withdraw() 
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    target_name = "TDLOG_ChessGame.exe" if os.name == 'nt' else "TDLOG_ChessGame"
    final_path = find_executable(base_dir, target_name)

    if final_path is None:
        messagebox.showerror("Erreur critique", "Exécutable introuvable.\nVeuillez compiler le projet C++.")
        sys.exit(1)

    variant_choice, mode_choice = ask_settings(root)
    print(f"Lancement : Variante={variant_choice}, Mode={mode_choice}")

    ai_count = 0 if mode_choice == "pvp" else 1

    root.deiconify()
    
    game = DisplayGame(root, variant_choice, final_path, nb_ai=ai_count)
    
    def on_closing():
        game.engine.close()
        game.root.destroy()
    root.protocol("WM_DELETE_WINDOW", on_closing)
    
    root.mainloop()