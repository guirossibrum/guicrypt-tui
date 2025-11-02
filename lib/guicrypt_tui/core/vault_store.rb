require_relative 'vault'
require 'json'

module GuicryptTui
  class VaultStore
    VAULTS_FILE = 'vaults.json'

    def initialize
      @vaults = load_vaults
    end

    def all
      @vaults
    end

    def add(vault)
      @vaults << vault
      save
    end

    def remove(id)
      @vaults.reject! { |v| v.id == id }
      save
    end

    def find(id)
      @vaults.find { |v| v.id == id }
    end

    private

    def load_vaults
      return [] unless File.exist?(VAULTS_FILE)

      data = JSON.parse(File.read(VAULTS_FILE))
      data.map do |h|
        vault = Vault.new(id: h['id'], name: h['name'], path: h['path'], mount_point: h['mount_point'])
        vault if vault.valid?
      end.compact
    end

    def save
      data = @vaults.map(&:to_h)
      File.write(VAULTS_FILE, JSON.pretty_generate(data))
    end
  end
end