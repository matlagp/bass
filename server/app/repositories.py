from .db_conn import DbConn
from .models import Node

class NodeRepository:
    def __init__(self):
        pass

    def create_database(self):
        with DbConn() as conn:
            conn.execute(
                'create table if not exists Nodes '
                '(id INTEGER PRIMARY KEY AUTOINCREMENT, '
                'ip TEXT NOT NULL)'
            )

    def all(self):
        with DbConn() as conn:
            return [
                Node(id, ip) for (id, ip)
                in conn.execute('select id, ip from Nodes')
            ]

    def create(self, node):
        with DbConn() as conn:
            cursor = conn.cursor()
            cursor.execute('insert into Nodes(ip) values(?)', (node.ip,))
            node.id = cursor.lastrowid
        return node

    def update(self, node):
        with DbConn() as conn:
            conn.execute(
                'update Nodes set ip = ? where id = ?',
                (node.ip, node.id)
            )
        return node

    def find(self, id):
        with DbConn() as conn:
            cmd = 'select * from Nodes where id = ? limit 1'
            return conn.execute(cmd, (id,)).fetchone()

    def delete(self, id):
        with DbConn() as conn:
            conn.execute('delete from Nodes where id = ?', (id,))
