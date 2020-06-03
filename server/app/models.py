class Node:
    def __init__(self, id=None, ip='0', volume=100):
        self.id = id
        self.ip = ip
        self.volume = volume

    def __str__(self):
        return str.format('Node(id={}, ip={}, volume={})', self.id, self.ip, self.volume)
