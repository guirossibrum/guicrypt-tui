require_relative '../test_helper'
require_relative '../../lib/guicrypt_tui/core/gocryptfs_service'

describe GuicryptTui::GocryptfsService do
  it 'checks if gocryptfs is installed' do
    expect(GuicryptTui::GocryptfsService.installed?).to be true
  end
end