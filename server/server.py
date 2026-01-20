import socket
import sqlite3
import os

HOST = '127.0.0.1'
PORT = 7778
DB_FILE = 'stomp_server.db'

def init_db():
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    
    c.execute('''CREATE TABLE IF NOT EXISTS users
                 (username TEXT PRIMARY KEY, password TEXT, registration_date TEXT)''')
    
    c.execute('''CREATE TABLE IF NOT EXISTS login_history
                 (id INTEGER PRIMARY KEY AUTOINCREMENT, 
                  username TEXT, 
                  login_time TEXT, 
                  logout_time TEXT)''')
    
    c.execute('''CREATE TABLE IF NOT EXISTS file_tracking
                 (id INTEGER PRIMARY KEY AUTOINCREMENT, 
                  username TEXT, 
                  filename TEXT, 
                  upload_time TEXT, 
                  game_channel TEXT)''')
    
    conn.commit()
    conn.close()
    print("Database initialized.")

def handle_client(conn):
    """Handle a single client connection"""
    try:
        data = b''
        while True:
            chunk = conn.recv(1024)
            if not chunk:
                break
            data += chunk
            if b'\0' in chunk: 
                break
        
        sql_command = data.decode('utf-8').replace('\0', '').strip()
        
        if not sql_command:
            return

        print(f"Executing SQL: {sql_command}")
        
        db_conn = sqlite3.connect(DB_FILE)
        cursor = db_conn.cursor()
        
        response = ""
        try:
            if sql_command.upper().startswith("SELECT"):
                cursor.execute(sql_command)
                rows = cursor.fetchall()
                if not rows:
                    response = "SUCCESS|No results"
                else:
                    response = "SUCCESS"
                    for row in rows:
                        response += "|" + str(row) 
            else:
                cursor.execute(sql_command)
                db_conn.commit()
                response = "SUCCESS"
                
        except sqlite3.Error as e:
            response = f"ERROR: {e}"
            print(f"SQL Error: {e}")
        finally:
            db_conn.close()
            
        conn.sendall((response + '\0').encode('utf-8'))
        
    except Exception as e:
        print(f"Connection Error: {e}")
    finally:
        conn.close()

def main():
    init_db()
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen()
        print(f"Python SQL Server listening on {HOST}:{PORT}")
        
        while True:
            conn, addr = s.accept()
            handle_client(conn)

if __name__ == '__main__':
    main()