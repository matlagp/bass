import sqlite3

class Node:
    def __init__(self, id=None, ip=None):
        self.id = id
        self.ip = ip

    def __str__(self):
        return str.format("Node(id={}, ip={})", self.id, self.ip)


class NodeRepository:
    def __init__(self):
        pass

    def create_database(self):
        with NodesConnection() as conn:
            conn.execute('create table if not exists Nodes '\
                        '(id INTEGER PRIMARY KEY AUTOINCREMENT, '\
                        'ip TEXT NOT NULL)')

    def all(self):
        with NodesConnection() as conn:
            return [ Node(id, ip) for (id, ip) in conn.execute('select id, ip from Nodes') ]

    def find(self, id):
        with NodesConnection() as conn:
            cmd = "select * from Nodes where id = ? limit 1"
            return conn.execute(cmd, id)[0]

    def create(self, node):
        with NodesConnection() as conn:
            cursor = conn.cursor()
            cursor.execute("insert into Nodes(ip) values(?)", (node.ip,))
            node.id = cursor.lastrowid
        return node

    def update(self, node):
        with NodesConnection() as conn:
            conn.execute('update Nodes set ip = ? where id = ?', (node.ip, node.id))
        return node

class NodesConnection:
    def __init__(self):
        self.conn = sqlite3.connect('nodes.db')

    def __enter__(self):
        return self.conn

    def __exit__(self, exc_type, exc_value, exc_traceback):
        print(exc_type, exc_value, exc_traceback)
        self.conn.commit()
        self.conn.close()
        return True
