require_relative 'gocryptfs_service'
require_relative 'keyring_service'
require 'fileutils'

module GuicryptTui
  class MountService
    def self.mounted?(mount_point)
      `mount`.include?(mount_point)
    end

    def self.create_mount_dir(mount_point)
      FileUtils.mkdir_p(mount_point) unless Dir.exist?(mount_point)
    end

    def self.mount(vault)
      password = KeyringService.get_password(vault.id)
      return false unless password

      create_mount_dir(vault.mount_point)
      GocryptfsService.mount(vault.path, vault.mount_point, password)
    end

    def self.unmount(vault)
      GocryptfsService.unmount(vault.mount_point) if mounted?(vault.mount_point)
    end

    def self.create_vault(path, password)
      GocryptfsService.create(path, password)
    end
  end
end