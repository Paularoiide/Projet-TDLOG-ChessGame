import tkinter as tk
import subprocess
import enum
import os
import time



COLORS = ["#F0D9B5", "#B58863", "#BBCB2B"]

"""STARTING_BOARDS = []
STARTING_BOARDS.append(
    [
            ["bR", "bN", "bB", "bQ", "bK", "bB", "bN", "bR"],
            ["bP", "bP", "bP", "bP", "bP", "bP", "bP", "bP"],
            ["--", "--", "--", "--", "--", "--", "--", "--"],
            ["--", "--", "--", "--", "--", "--", "--", "--"],
            ["--", "--", "--", "--", "--", "--", "--", "--"],
            ["--", "--", "--", "--", "--", "--", "--", "--"],
            ["wP", "wP", "wP", "wP", "wP", "wP", "wP", "wP"],
            ["wR", "wN", "wB", "wQ", "wK", "wB", "wN", "wR"],
        ]
)"""

BOARD_SIZE = 600
SQUARE_SIZE = BOARD_SIZE // 8

PIECES = {
    "P": "♙", "R": "♖", "N": "♘", "B": "♗", "Q": "♕", "K": "♔",
    "p": "♟", "r": "♜", "n": "♞", "b": "♝", "q": "♛", "k": "♚",
    "-": " " 
}



class Variante(enum.Enum):
    CLASSIC = 0

class Engine():
    def __init__(self, engine:str, nb_ai:int =0):
        if not os.path.exists(engine):
            raise FileNotFoundError(f"Moteur introuvable à {engine}")
        self.process = subprocess.Popen([engine], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1)

    def send_request(self, request: str|None):
        if self.process.stdin :
            if request is not None:
                self.process.stdin.write(request+"\n")
                self.process.stdin.flush()
            return self.read_board()
            
        
    def close(self):
        if self.process:
            self.process.terminate()

    def read_board(self):
        if self.process.stdout:
            lines = []
            for _ in range(8):
                lines.append(self.process.stdout.readline().strip())
            self.process.stdout.flush()
            return lines
        else:
            print(f"Board couldn't be read.\n")



class Move():
    def __init__(self, canvas, move_func):
        self.canvas = canvas
        self.move = move_func
        self.selected = None
        self.canvas.bind("<Button-1>", self.click)
        self.highlighted = None

    def highlight(self, col, row):
        x0 = col * SQUARE_SIZE
        y0 = row * SQUARE_SIZE
        x1 = x0 + SQUARE_SIZE
        y1 = y0 + SQUARE_SIZE
        self.highlighted = self.canvas.create_rectangle(x0, y0, x1, y1, outline=COLORS[-1], width=4)

    def click(self, event):
        col, row = event.x//SQUARE_SIZE, event.y//SQUARE_SIZE
        pos = f"{chr(col + ord('a'))}{8 - row}"

        if self.selected is None:
            self.selected = pos
            self.highlight(col, row)
        else:
            if self.selected!=pos:
                self.move(self.selected, pos)
            self.selected = None
            self.canvas.delete(self.highlighted)
            self.highlighted = None



        


class Board():
    def __init__(self, var: Variante = Variante.CLASSIC):
        self.board = [[] for _ in range(8)]

    def __getitem__(self, pos: str):
        assert(len(pos)==2)
        col, row = pos[0], pos[1]
        col = ord(col) - ord('a')
        row = 8 - int(row)
        assert(0<=col<8 and 0<=row<8), "index out of bounds"
        piece = self.board[row][col] 
        return None if piece == "--" else piece
    
    def __setitem__(self, pos: str, val: str):
        assert(len(pos)==2)
        col, row = pos[0], pos[1]
        col = ord(col) - ord('a')
        print(row)
        print(type(row))
        row = 8 - int(row)
        assert(0<=col<8 and 0<=row<8), "index out of bounds"
        self.board[row][col] = val

    def update(self, board_output: str):
        assert(len(board_output)==8), f"Wrong board output from engine"
        for i in range(8):
            self.board[i] = (board_output[i].split())[:8]

    """def move(self, pos_i: str, pos_f: str):
        replaced = self[pos_f]
        self[pos_f] = self[pos_i]
        self[pos_i] = "--"
        return None if replaced == "--" else replaced"""
    


class DisplayGame():
    def __init__(self, root, variante: Variante, engine: str, nb_ai: int =0):
        self.root = root
        self.board = Board(variante)
        self.engine = Engine(engine, nb_ai)
        assert(0<=nb_ai<=2), "Number of AI must be 0, 1 or 2!"
        self.gamemode = "PvP" if nb_ai==0 else "PvAI" if nb_ai==1 else "AIvAI"
        title = "Chess Game : " + self.gamemode
        self.root.title(title)
        self.canvas = tk.Canvas(root, width=BOARD_SIZE, height=BOARD_SIZE)
        self.canvas.pack()
        self.move_handler = Move(self.canvas, self.submit_move)
        self.act(self.engine.read_board())

    def draw_board(self):
        self.canvas.delete("all")
        col_ind = 0
        for row in range(8):
            for col in range(8):
                x0 = col * SQUARE_SIZE
                y0 = row * SQUARE_SIZE
                x1 = x0 + SQUARE_SIZE
                y1 = y0 + SQUARE_SIZE
                self.canvas.create_rectangle(x0, y0, x1, y1, fill=COLORS[col_ind], outline="")
                pos = f"{chr(col+ord('a'))}{8-row}"
                piece = self.board[pos]
                if piece is not None:
                    self.canvas.create_text(
                        x0 + SQUARE_SIZE // 2, y0 + SQUARE_SIZE // 2, 
                        text=PIECES[piece], font=("Arial", 32), fill="black")
                col_ind = (col_ind+1)%2
            col_ind = (col_ind+1)%2

    def submit_move(self, pos1, pos2):
        command = f"{pos1} {pos2}"
        self.ask_engine(command)
        if self.gamemode=="PvAI":
            self.ask_engine()

    def ask_engine(self, command=None):
        if command is None:
            time.sleep(0.5)
        answer = self.engine.send_request(command)
        self.act(answer)

    def act(self, answer: str):
            self.board.update(answer)
            self.draw_board()



if __name__== "__main__":
    root = tk.Tk()
    folder = "./TDLOG_ChessGame/build"
    if os.name == 'nt':
        engine = os.path.join(folder, "TDLOG_ChessGame.exe")
    else:
        engine = os.path.join(folder, "TDLOG_ChessGame")

    assert(os.path.exists(engine)), f"Engine not found"
    game = DisplayGame(root, Variante.CLASSIC, engine, nb_ai = 0)

    def on_closing():
        game.engine.close()
        game.root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()
