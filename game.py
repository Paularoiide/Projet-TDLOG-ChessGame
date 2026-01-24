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
    def __init__(self, engine_path: str, variant: str, nb_ai: int = 0, ai_depth = [5, 5]):
        if not os.path.exists(engine_path):
            raise FileNotFoundError(f"Moteur introuvable à : {engine_path}")
        
        try:
            st = os.stat(engine_path)
            os.chmod(engine_path, st.st_mode | stat.S_IEXEC)
        except Exception:
            pass

        cmd = [engine_path, variant] 

        if nb_ai == 1:
            mode_arg = "PvAI"
            cmd.append(mode_arg)
            cmd.append(str(ai_depth[0]))
        elif nb_ai == 2:
            mode_arg = "AIvAI"
            cmd.append(mode_arg)
            cmd.append(str(ai_depth[0]))
            cmd.append(str(ai_depth[1]))
        else:
            mode_arg = "PvP"
            cmd.append(mode_arg)
                
        
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
        """Envoie une commande de JEU et attend une réponse."""
        if self.process.stdin:
            if request is not None:
                try:
                    self.process.stdin.write(request + "\n")
                    self.process.stdin.flush()
                except BrokenPipeError:
                    print("Erreur : Le moteur C++ s'est arrêté.")
                    return "ERR", []
            return self.read_board()
        return "ERR", []
            
    def close(self):
        if self.process:
            self.process.terminate()

    def read_board(self):
        """Lit la réponse du C++."""
        try:
            prefix = self.process.stdout.readline()
            if not prefix: return "END", []
            
            prefix = prefix.strip()
            if prefix.startswith("POS"):
                parts = prefix.split()
                return "POS", parts[1:]
            elif prefix == "PRO":
                return "PRO", []

            elif prefix in ["VAL", "ILL"]:
                lines = []
                while len(lines) < 8:
                    line = self.process.stdout.readline()
                    if not line: break 
                    stripped = line.strip()
                    if not stripped: continue
                    lines.append(stripped)
                return prefix, lines
            
            elif prefix == "END":
                return "END", []
            
            else:
                print(f"DEBUG: Reçu inconnu '{prefix}'")
                return "UNK", []
                
        except Exception as e:
            print(f"Erreur lecture : {e}")
            return "ERR", []

# --- LOGIQUE UI ---
class Move():
    def __init__(self, canvas, game_instance):
        self.canvas = canvas
        self.game = game_instance 
        self.selected = None
        self.highlighted = None
        self.suggestion_rects = []
        self.canvas.bind("<Button-1>", self.click)

    def highlight(self, col, row):
        x0 = col * SQUARE_SIZE
        y0 = row * SQUARE_SIZE
        x1 = x0 + SQUARE_SIZE
        y1 = y0 + SQUARE_SIZE
        self.highlighted = self.canvas.create_rectangle(x0, y0, x1, y1, outline=COLORS[-1], width=4)

    def show_suggestions(self, moves):
        self.clear_suggestions()
        for sq in moves:
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
        if not (0 <= col < 8 and 0 <= row < 8): return
        pos = f"{chr(col + ord('a'))}{8 - row}"
        target_piece = self.game.board[pos]
        
        if self.selected is None:
            if not target_piece: return
            self.select_piece(pos, col, row)

        else:
            source_piece = self.game.board[self.selected]
            if target_piece and source_piece:
                if source_piece.isupper() == target_piece.isupper():
                    self.clear_suggestions()
                    if self.highlighted: self.canvas.delete(self.highlighted)
                    self.select_piece(pos, col, row)
                    return

            self.clear_suggestions()
            if self.selected != pos:
                self.game.submit_move(self.selected, pos)
            
            self.selected = None
            if self.highlighted:
                self.canvas.delete(self.highlighted)
            self.highlighted = None

    def select_piece(self, pos, col, row):
        """Helper pour sélectionner proprement"""
        self.selected = pos
        self.highlight(col, row)
        response = self.game.engine.send_request(f"POS {pos}")
        if response and response[0] == "POS":
            self.show_suggestions(response[1])

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
        new_board = []
        for line in board_output:
            parts = line.split()
            if len(parts) != 8: return 
            new_board.append(parts)
        self.board = new_board

    def get_raw_data(self):
        return [row[:] for row in self.board]

class DisplayGame():
    def __init__(self, root, variant_str: str, engine_path: str, nb_ai: int = 0, ai_depth = [5, 5]):
        self.root = root
        self.engine = Engine(engine_path, variant_str, nb_ai, ai_depth)
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

        initial_board = []
        try:
            for _ in range(8):
                line = self.engine.process.stdout.readline().strip()
                if line: initial_board.append(line)
        except:
            pass
        
        self.board.update(initial_board)
        self.draw_board()

        if self.gamemode == "AIvAI":
            self.play_AIvAI()


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

    def wait_prom(self, pos):
        """Affiche une popup graphique avec des images sur la case cible."""
        
        # 1. Calcul de la position de la case cible à l'écran
        col = ord(pos[0]) - ord('a')
        row = 8 - int(pos[1]) # 0 en haut, 7 en bas
        
        # Coordonnées globales de la fenêtre principale + offset de la case
        # On ajoute un petit ajustement pour centrer la popup (largeur estimée ~200px)
        # SQUARE_SIZE est 600 // 8 = 75
        popup_width = 4 * 60  # 4 boutons d'environ 60px
        popup_height = 60
        
        # Position X : Racine X + Case X - (Moitié popup) + (Moitié Case)
        screen_x = self.root.winfo_rootx() + (col * SQUARE_SIZE) - (popup_width // 2) + (SQUARE_SIZE // 2)
        screen_y = self.root.winfo_rooty() + (row * SQUARE_SIZE)
        
        # Si on est tout en haut (Blancs), on affiche la popup un peu plus bas pour pas sortir de l'écran
        if row == 0: screen_y += 10
        # Si on est tout en bas (Noirs), on affiche un peu plus haut
        else: screen_y -= popup_height

        # 2. Création de la fenêtre
        top = tk.Toplevel(self.root)
        top.geometry(f"+{int(screen_x)}+{int(screen_y)}")
        top.overrideredirect(True) # Enlève la barre de titre (style popup moderne)
        top.attributes("-topmost", True) # Toujours au premier plan
        
        # Variable pour le résultat (par défaut reine 'q')
        choice_var = tk.StringVar(value="q")

        def select(code):
            choice_var.set(code)
            top.destroy()
        
        # 3. Choix des images (Blanches ou Noires ?)
        # Si pos fini par '8', c'est les blancs qui promeuvent
        is_white = (pos[1] == '8')
        
        # Liste des choix : (Code C++, Clé Image)
        options = [
            ("q", "Q" if is_white else "q"),
            ("r", "R" if is_white else "r"),
            ("b", "B" if is_white else "b"),
            ("n", "N" if is_white else "n")
        ]

        # 4. Création des boutons avec images
        for code, img_key in options:
            if img_key in self.piece_images:
                img = self.piece_images[img_key]
                btn = tk.Button(top, image=img, command=lambda c=code: select(c), 
                                bg="#f0f0f0", activebackground="#BBCB2B", bd=1)
                btn.pack(side=tk.LEFT, padx=2, pady=2)
            else:
                # Fallback texte si image manquante
                tk.Button(top, text=code.upper(), command=lambda c=code: select(c)).pack(side=tk.LEFT)

        # 5. Attente bloquante
        top.grab_set() 
        self.root.wait_window(top)
        
        return choice_var.get()

    def submit_move(self, pos1, pos2):
        piece = self.board[pos1]
        if not piece: return

        is_pawn = (piece.lower() == 'p')
        is_last_rank = (pos2[1] == '1' or pos2[1] == '8')
        
        promo_suffix = ""
        
        if is_pawn and is_last_rank:
            promo_suffix = self.wait_prom(pos2)
            if not promo_suffix: promo_suffix = " q"

        cmd = f"MOV {pos1} {pos2} {promo_suffix}"
        prefix, data = self.ask_engine(cmd)
        
        if prefix == "VAL" and self.gamemode == "PvAI":
            self.root.update()
            self.root.after(50, self.trigger_ai_turn)

    def trigger_ai_turn(self):
        prefix, data = self.engine.read_board()
        if prefix == "VAL":
            self.act(data)
        elif prefix == "END":
            messagebox.showinfo("Fin de partie", "Partie terminée !")

    def ask_engine(self, command=None):
        prefix, data = self.engine.send_request(command)
        if prefix in ["VAL", "ILL"] and data:
            self.act(data)
        return prefix, data

    def act(self, answer: list):
        self.board.update(answer)
        self.draw_board()

    def play_AIvAI(self):
        prefix, data = self.engine.read_board() 
        if prefix == "VAL":
            self.act(data)
            self.root.after(100, self.play_AIvAI)
        elif prefix == "END":
            print("Fin de partie AI vs AI")





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
    var_mode = tk.StringVar(value="PvP")
    
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
    tk.Radiobutton(frame_mode, text="Joueur vs Joueur (PvP)", variable=var_mode, value="PvP").pack(side=tk.LEFT, padx=10)
    tk.Radiobutton(frame_mode, text="Joueur vs IA (PvE)", variable=var_mode, value="PvAI").pack(side=tk.LEFT, padx=10)
    
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

    ai_count = 0 if mode_choice == "PvP" else 1 if mode_choice == "PvAI" else 2

    root.deiconify()
    
    game = DisplayGame(root, variant_choice, final_path, nb_ai=ai_count)
    
    def on_closing():
        game.engine.close()
        game.root.destroy()
    root.protocol("WM_DELETE_WINDOW", on_closing)
    
    root.mainloop()