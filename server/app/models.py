class Node:
    def __init__(self, id=None, ip='0', volume=100, bass=0, mid=0, trebble=0):
        self.id = id
        self.ip = ip
        self.volume = volume
        self.bass = bass
        self.mid = mid
        self.trebble = trebble

    def __str__(self):
        return f"Node(id={self.id}, ip={self.ip}, volume={self.volume}, bass={self.bass}, mid={self.mid}, trebble={self.trebble})"

    @property
    def name(self):
        return f"Node #{self.id}"
