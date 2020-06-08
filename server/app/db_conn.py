import sqlite3


class DbConn:
    def __init__(self):
        self.conn = sqlite3.connect('bass.db')

    def __enter__(self):
        return self.conn

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.conn.commit()
        self.conn.close()
