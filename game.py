import tkinter as tk
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
    "P": "wp.png",
    "R": "wr.png",
    "N": "wn.png",
    "B": "wb.png",
    "Q": "wq.png",
    "K": "wk.png",
    "p": "bp.png",
    "r": "br.png",
    "n": "bn.png",
    "b": "bb.png",
    "q": "bq.png",
    "k": "bk.png",
    "A": "wprincess.png",
    "a": "bprincess.png",
    "E": "wempress.png",
    "e": "bempress.png",
    "G": "wgrasshopper.png",
    "g": "bgrasshopper.png",
    "H": "wnightrider.png",
    "h": "bnightrider.png",
}


class Variante(enum.Enum):
    CLASSIC = 0
    FAIRY = 1

# --- MOTEUR ---
class Engine():
    def __init__(self, engine_path: str, variant: str, nb_ai: int = 0):
        if not os.path.exists(engine_path):
            raise FileNotFoundError(f"Moteur introuvable à : {engine_path}")
        
        # Force les permissions d'exécution
        try:
            st = os.stat(engine_path)
            os.chmod(engine_path, st.st_mode | stat.S_IEXEC)
        except Exception:
            pass
        
        cmd = [engine_path, variant]
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
        """Envoie une commande et attend la réponse."""
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
            
    def close(self):
        if self.process:
            self.process.terminate()

    def read_board(self):
        """Lit 8 lignes valides depuis le C++."""
        lines = []
        # On lit jusqu'à avoir 8 lignes, ou que le processus meurt
        while len(lines) < 8:
            line = self.process.stdout.readline()
            if not line: break # Fin du flux (EOF)
            stripped = line.strip()
            if not stripped: continue # Ignore les lignes vides
            lines.append(stripped)
        return lines

# --- LOGIQUE UI ---
class Move():
    def __init__(self, canvas, move_func):
        self.canvas = canvas
        self.move = move_func
        self.selected = None
        self.highlighted = None
        self.canvas.bind("<Button-1>", self.click)

    def highlight(self, col, row):
        x0 = col * SQUARE_SIZE
        y0 = row * SQUARE_SIZE
        x1 = x0 + SQUARE_SIZE
        y1 = y0 + SQUARE_SIZE
        self.highlighted = self.canvas.create_rectangle(x0, y0, x1, y1, outline=COLORS[-1], width=4)

    def click(self, event):
        col, row = event.x // SQUARE_SIZE, event.y // SQUARE_SIZE
        pos = f"{chr(col + ord('a'))}{8 - row}"
        if self.selected is None:
            self.selected = pos
            self.highlight(col, row)
        else:
            if self.selected != pos:
                self.move(self.selected, pos)
            self.selected = None
            if self.highlighted:
                self.canvas.delete(self.highlighted)
            self.highlighted = None

class Board():
    def __init__(self, var: Variante = Variante.CLASSIC):
        # Initialisation avec des lignes vides, mais sera rempli immédiatement
        self.board = [[] for _ in range(8)]

    def __getitem__(self, pos: str):
        if len(pos) != 2: return None
        col = ord(pos[0]) - ord('a')
        row = 8 - int(pos[1])
        if not (0 <= col < 8 and 0 <= row < 8): return None
        
        # Sécurité anti-crash
        if row >= len(self.board) or col >= len(self.board[row]):
            return None
            
        piece = self.board[row][col] 
        return None if piece == "-" else piece

    def update(self, board_output: list):
        """Met à jour le plateau de manière sécurisée."""
        if len(board_output) != 8:
            print(f"DEBUG: Reçu {len(board_output)} lignes au lieu de 8. Ignoré.")
            return 

        # On construit d'abord un plateau temporaire
        new_board = []
        for line in board_output:
            parts = line.split()
            # Si une ligne n'a pas exactement 8 cases, c'est une erreur de lecture
            if len(parts) != 8:
                print(f"DEBUG: Ligne corrompue reçue : '{line}'")
                return 
            new_board.append(parts)
        
        # Si tout est bon, on remplace
        self.board = new_board

    def get_raw_data(self):
        """Retourne une copie brute des données pour comparaison"""
        return [row[:] for row in self.board]

class DisplayGame():
    def __init__(self, root, variant_str: str, engine_path: str, nb_ai: int = 0):
        self.root = root
        self.board = Board(Variante.FAIRY if variant_str == "fairy" else Variante.CLASSIC)
        self.engine = Engine(engine_path, variant_str, nb_ai)
        self.nb_ai = nb_ai
        self.gamemode = "PvP" if nb_ai == 0 else "PvAI" if nb_ai == 1 else "AIvAI"
        
        self.root.title(f"Chess : {self.gamemode}")
        self.canvas = tk.Canvas(root, width=BOARD_SIZE, height=BOARD_SIZE)
        self.canvas.pack()
        self.move_handler = Move(self.canvas, self.submit_move)

        # Display the images for pieces
        self.piece_images = {}
        asset_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "assets")
        for piece, filename in PIECE_IMAGES.items():
            path = os.path.join(asset_dir, filename)
            if os.path.exists(path):

                img = tk.PhotoImage(file=path)
                scale = img.width() // SQUARE_SIZE
                if scale > 1:
                    img = img.subsample(scale, scale)

                self.piece_images[piece] = img

        # Lecture état initial
        self.act(self.engine.read_board())


    def draw_board(self):
        self.canvas.delete("all")
        for row in range(8):
            for col in range(8):
                x0, y0 = col * SQUARE_SIZE, row * SQUARE_SIZE
                x1, y1 = x0 + SQUARE_SIZE, y0 + SQUARE_SIZE
                
                self.canvas.create_rectangle(x0, y0, x1, y1, fill=COLORS[(row + col) % 2], outline="")
                
                # Récupération sécurisée via __getitem__
                pos = f"{chr(col + ord('a'))}{8 - row}"
                piece = self.board[pos]
                
                if piece in self.piece_images:
                    self.canvas.create_image(x0 + SQUARE_SIZE//2, y0 + SQUARE_SIZE//2, anchor=tk.CENTER, image=self.piece_images[piece])

    def submit_move(self, pos1, pos2):
        # Logique promotion
        piece = self.board[pos1]
        # Petite sécurité si clic sur case vide
        if not piece: return

        is_promo = (piece == 'P' and pos2[1] == '8') or (piece == 'p' and pos2[1] == '1')
        cmd = f"{pos1} {pos2}" + (" q" if is_promo else "")
        print(f"Coup joueur: {cmd}")

        # 1. Sauvegarde de l'état actuel
        previous_state = self.board.get_raw_data()

        # 2. Envoi du coup humain
        self.ask_engine(cmd)
        
        # 3. Vérification : Si le plateau n'a pas changé, le coup était illégal
        current_state = self.board.get_raw_data()
        if previous_state == current_state:
            print(">> Coup illégal détecté (plateau inchangé). L'IA ne joue pas.")
            return

        # 4. Si coup valide et mode PvAI, l'IA joue
        if self.gamemode == "PvAI":
            self.root.update()
            # On envoie None pour lire la réponse de l'IA (qui joue automatiquement en C++)
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

# Fonction pour afficher la popup de choix
def ask_variant(root):
    choice = tk.StringVar(value="classic") # Valeur par défaut
    
    # Création de la fenêtre secondaire
    dialog = tk.Toplevel(root)
    dialog.title("Configuration")
    dialog.geometry("300x150")
    
    tk.Label(dialog, text="Choisissez le mode de jeu :", font=("Arial", 12)).pack(pady=10)
    
    # Bouton Classique
    def set_classic():
        choice.set("classic")
        dialog.destroy()
        
    tk.Button(dialog, text="Classique", command=set_classic, width=20).pack(pady=5)
    
    # Bouton Féerique
    def set_fairy():
        choice.set("fairy")
        dialog.destroy()
        
    tk.Button(dialog, text="Féerique (Fairy)", command=set_fairy, width=20).pack(pady=5)
    
    # Important : on attend que la fenêtre soit fermée avant de continuer
    root.wait_window(dialog)
    return choice.get()

# --- BLOC PRINCIPAL ---
if __name__== "__main__":
    root = tk.Tk()
    root.withdraw() # On cache la fenêtre principale pour l'instant
    
    base_dir = os.path.dirname(os.path.abspath(__file__))
    target_name = "TDLOG_ChessGame.exe" if os.name == 'nt' else "TDLOG_ChessGame"
    final_path = find_executable(base_dir, target_name)

    if final_path is None:
        print("ERREUR : Exécutable introuvable.")
        sys.exit(1)

    # 1. On demande à l'utilisateur quel mode il veut
    selected_mode = ask_variant(root)
    print(f"Mode choisi : {selected_mode}")

    # 2. On réaffiche la fenêtre principale
    root.deiconify()
    
    # 3. On lance le jeu
    # IMPORTANT : nb_ai=0 pour le mode Joueur vs Joueur (PvP)
    game = DisplayGame(root, selected_mode, final_path, nb_ai=1)
    
    def on_closing():
        game.engine.close()
        game.root.destroy()
    root.protocol("WM_DELETE_WINDOW", on_closing)
    
    root.mainloop()