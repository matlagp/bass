class Node:
    def __init__(self, id=None, ip=None):
        self.id = id
        self.ip = ip

    def __str__(self):
        return str.format('Node(id={}, ip={})', self.id, self.ip)
