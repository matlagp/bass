class Node:
    def __init__(self, id=None, ip='0', name=None, volume=100, bass=0, mid=0, treble=0):
        self.id = id
        self._name = name
        self.ip = ip
        self.volume = volume
        self.bass = bass
        self.mid = mid
        self.treble = treble

    def __str__(self):
        return f"Node(id={self.id}, ip={self.ip}, name={self.name}, volume={self.volume}, bass={self.bass}, mid={self.mid}, treble={self.treble})"

    @property
    def hex_id(self):
        return "{0:08X}".format(self.id)

    @property
    def name(self):
        if self._name is None:
            return f"Node #{self.hex_id}"
        else:
            return self._name

    @name.setter
    def name(self, new_name):
        self._name = new_name
