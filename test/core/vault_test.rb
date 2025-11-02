require_relative '../test_helper'
require_relative '../../lib/guicrypt_tui/core/vault'

describe GuicryptTui::Vault do
  let(:vault) { GuicryptTui::Vault.new(id: 1, name: 'test', path: '.', mount_point: '/tmp') }

  it 'has correct attributes' do
    expect(vault.id).to eq(1)
    expect(vault.name).to eq('test')
    expect(vault.path).to eq('.')
    expect(vault.mount_point).to eq('/tmp')
  end

  it 'converts to hash' do
    expect(vault.to_h).to eq({ id: 1, name: 'test', path: '.', mount_point: '/tmp' })
  end

  it 'is valid if path exists' do
    expect(vault.valid?).to be true
  end
end