#!/usr/bin/env python3
"""
Basic Python Server for STOMP Assignment – Stage 3.3

IMPORTANT:
DO NOT CHANGE the server name or the basic protocol.
Students should EXTEND this server by implementing
the methods below.
"""

import socket
import sys
import os
import threading
import sqlite3


SERVER_NAME = "STOMP_PYTHON_SQL_SERVER"  # DO NOT CHANGE!
DB_FILE = "stomp_server.db"              # DO NOT CHANGE!


def recv_null_terminated(sock: socket.socket) -> str:
    data = b""
    while True:
        chunk = sock.recv(1024)
        if not chunk:
            return ""
        data += chunk
        if b"\0" in data:
            msg, _ = data.split(b"\0", 1)
            return msg.decode("utf-8", errors="replace")


def init_database():
    try:
        conn = sqlite3.connect(DB_FILE)
        cursor = conn.cursor()
        
        # 1. Users table
        cursor.execute('''CREATE TABLE IF NOT EXISTS users
                          (username TEXT PRIMARY KEY,
                           password TEXT,
                           registration_date TEXT)''')
        
        # 2. Login History table
        cursor.execute('''CREATE TABLE IF NOT EXISTS login_history
                          (id INTEGER PRIMARY KEY AUTOINCREMENT,
                           username TEXT,
                           login_time TEXT,
                           logout_time TEXT)''')
        
        # 3. File Tracking table (Reports)
        cursor.execute('''CREATE TABLE IF NOT EXISTS file_tracking
                          (id INTEGER PRIMARY KEY AUTOINCREMENT,
                           username TEXT,
                           filename TEXT,
                           upload_time TEXT,
                           game_channel TEXT)''')
        
        conn.commit()
        conn.close()
        print(f"[{SERVER_NAME}] Database initialized successfully.")
    except Exception as e:
        print(f"[{SERVER_NAME}] Error initializing database: {e}")


def execute_sql_command(sql_command: str) -> str:
    try:
        with sqlite3.connect(DB_FILE) as conn:
            cursor = conn.cursor()
            cursor.execute(sql_command)
            conn.commit()
            return "SUCCESS"
    except sqlite3.Error as e:
        return f"ERROR: {e}"


def execute_sql_query(sql_query: str) -> str:
    try:
        with sqlite3.connect(DB_FILE) as conn:
            cursor = conn.cursor()
            cursor.execute(sql_query)
            rows = cursor.fetchall()
            
            # Format the result for Java parsing (pipe separated)
            if not rows:
                return "SUCCESS|No results"
            
            response = "SUCCESS"
            for row in rows:
                response += "|" + str(row)
            return response
            
    except sqlite3.Error as e:
        return f"ERROR: {e}"


def handle_client(client_socket: socket.socket, addr):
    print(f"[{SERVER_NAME}] Client connected from {addr}")

    try:
        while True:
            message = recv_null_terminated(client_socket)
            if message == "":
                break

            print(f"[{SERVER_NAME}] Received SQL: {message}")

            # --- FIX STARTS HERE ---
            
            response = ""
            # Check if it is a read (SELECT) or write (INSERT/UPDATE/DELETE) operation
            if message.strip().upper().startswith("SELECT"):
                response = execute_sql_query(message)
            else:
                response = execute_sql_command(message)
            
            # Send the actual database response back to Java
            # (The Java code expects null-terminated strings)
            client_socket.sendall(response.encode("utf-8") + b"\0")
            
            # --- FIX ENDS HERE ---

    except Exception as e:
        print(f"[{SERVER_NAME}] Error handling client {addr}: {e}")
    finally:
        try:
            client_socket.close()
        except Exception:
            pass
        print(f"[{SERVER_NAME}] Client {addr} disconnected")


def start_server(host="127.0.0.1", port=7778):
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind((host, port))
        server_socket.listen(5)
        print(f"[{SERVER_NAME}] Server started on {host}:{port}")
        print(f"[{SERVER_NAME}] Waiting for connections...")

        while True:
            client_socket, addr = server_socket.accept()
            t = threading.Thread(
                target=handle_client,
                args=(client_socket, addr),
                daemon=True
            )
            t.start()

    except KeyboardInterrupt:
        print(f"\n[{SERVER_NAME}] Shutting down server...")
    finally:
        try:
            server_socket.close()
        except Exception:
            pass


if __name__ == "__main__":
    port = 7778
    if len(sys.argv) > 1:
        raw_port = sys.argv[1].strip()
        try:
            port = int(raw_port)
        except ValueError:
            print(f"Invalid port '{raw_port}', falling back to default {port}")
    init_database()
    start_server(port=port)
