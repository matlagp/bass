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
                'name TEXT,'
                'volume INT NOT NULL DEFAULT 100,'
                'bass REAL NOT NULL DEFAULT 0,'
                'mid REAL NOT NULL DEFAULT 0,'
                'treble REAL NOT NULL DEFAULT 0)'
            )

    def all(self):
        with DbConn() as conn:
            return [
                Node(id, ip, name, volume, bass, mid, treble) for (id, ip, name, volume, bass, mid, treble)
                in conn.execute('select id, ip, name, volume, bass, mid, treble from Nodes')
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
                'update Nodes set ip = ?, name = ?, volume = ?, bass = ?, mid = ?, treble = ? where id = ?',
                (node.ip, node.name, node.volume, node.bass, node.mid, node.treble, node.id)
            )
        return node

    def find(self, id):
        with DbConn() as conn:
            cmd = 'select id, ip, name, volume, bass, mid, treble from Nodes where id = ? limit 1'
            params = conn.execute(cmd, (id,)).fetchone()
            return Node(*params)

    def delete(self, id):
        with DbConn() as conn:
            conn.execute('delete from Nodes where id = ?', (id,))
