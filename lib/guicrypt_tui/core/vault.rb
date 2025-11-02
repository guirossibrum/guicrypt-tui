module GuicryptTui
  class Vault
    attr_reader :id, :name, :path, :mount_point

    def initialize(id:, name:, path:, mount_point:)
      @id = id
      @name = name
      @path = path
      @mount_point = mount_point
    end

    def to_h
      { id: @id, name: @name, path: @path, mount_point: @mount_point }
    end

    def valid?
      File.exist?(@path) && File.directory?(@path)
    end
  end
end