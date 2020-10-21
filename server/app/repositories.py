from .db_conn import DbConn
from .models import Node


class NodeRepository:
    def __init__(self):
        pass

    def create_database(self):
        with DbConn() as conn:
            conn.execute(
                'create table if not exists Nodes'
                '(id INTEGER PRIMARY KEY,'
                'ip TEXT NOT NULL,'
                'volume INT NOT NULL DEFAULT 100,'
                'bass REAL NOT NULL DEFAULT 0,'
                'mid REAL NOT NULL DEFAULT 0,'
                'trebble REAL NOT NULL DEFAULT 0)'
            )

    def all(self):
        with DbConn() as conn:
            return [
                Node(id, ip, volume, bass, mid, trebble) for (id, ip, volume, bass, mid, trebble)
                in conn.execute('select id, ip, volume, bass, mid, trebble from Nodes')
            ]

    def create(self, node):
        with DbConn() as conn:
            conn.execute(
                'insert into Nodes(id, ip) values(?, ?) '
                'on conflict(id) do update '
                'set ip = excluded.ip',
                (node.id, node.ip)
            )
        return node

    def update(self, node):
        with DbConn() as conn:
            conn.execute(
                'update Nodes set ip = ?, volume = ?, bass = ?, mid = ?, trebble = ? where id = ?',
                (node.ip, node.volume, node.bass, node.mid, node.trebble, node.id)
            )
        return node

    def find(self, id):
        with DbConn() as conn:
            cmd = 'select id, ip, volume, bass, mid, trebble from Nodes where id = ? limit 1'
            params = conn.execute(cmd, (id,)).fetchone()
            return Node(*params)

    def delete(self, id):
        with DbConn() as conn:
            conn.execute('delete from Nodes where id = ?', (id,))
