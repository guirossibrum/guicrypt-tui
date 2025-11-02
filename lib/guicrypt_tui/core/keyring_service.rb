require 'keyring'

module GuicryptTui
  class KeyringService
    def self.store_password(vault_id, password)
      keyring = Keyring.new
      keyring.set_password('guicrypt-tui', vault_id.to_s, password)
    end

    def self.get_password(vault_id)
      keyring = Keyring.new
      keyring.get_password('guicrypt-tui', vault_id.to_s)
    end
  end
end