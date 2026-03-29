import { Link, useLocation } from 'react-router-dom'

function Layout({ children, user, onLogout }) {
  const location = useLocation()

  const navItems = [
    { path: '/', label: '📊 Dashboard', icon: '📊' },
    { path: '/upload', label: '📁 Upload Task', icon: '📁' },
  ]

  return (
    <div>
      {/* Header */}
      <div style={{ background: 'linear-gradient(135deg, #1a1a2e 0%, #16213e 100%)', color: 'white', padding: '0 24px', height: '70px', display: 'flex', alignItems: 'center', justifyContent: 'space-between', position: 'sticky', top: 0, zIndex: 100 }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <div style={{ fontSize: '32px' }}>🚀</div>
          <div style={{ fontSize: '20px', fontWeight: 'bold' }}>Web <span style={{ color: '#4ade80' }}>Tool</span> Automation</div>
        </div>
        <div style={{ display: 'flex', gap: '32px' }}>
          {navItems.map(item => (
            <Link key={item.path} to={item.path} style={{ color: location.pathname === item.path ? 'white' : '#a0a0a0', textDecoration: 'none', fontWeight: '500', padding: '8px 0', borderBottom: location.pathname === item.path ? '2px solid #4ade80' : 'none' }}>
              {item.label}
            </Link>
          ))}
        </div>
        <div style={{ display: 'flex', alignItems: 'center', gap: '20px' }}>
          <div style={{ fontSize: '20px', cursor: 'pointer' }}>🔔</div>
          <div style={{ width: '40px', height: '40px', background: 'linear-gradient(135deg, #4ade80, #22c55e)', borderRadius: '50%', display: 'flex', alignItems: 'center', justifyContent: 'center', fontWeight: 'bold', cursor: 'pointer' }}>
            {user?.name?.charAt(0) || 'U'}
          </div>
          <button onClick={onLogout} style={{ background: 'none', border: '1px solid #a0a0a0', color: 'white', padding: '6px 12px', borderRadius: '8px', cursor: 'pointer' }}>Logout</button>
        </div>
      </div>

      {/* Main Content */}
      <div style={{ maxWidth: '1400px', margin: '0 auto', padding: '24px' }}>
        {children}
      </div>

      {/* Footer */}
      <div style={{ textAlign: 'center', padding: '24px', color: '#666', fontSize: '12px', borderTop: '1px solid #e2e8f0', marginTop: '20px' }}>
        🚀 Web Tool Automation | Jira · Git · SharePoint · Confluence | Last sync: 2 minutes ago
      </div>
    </div>
  )
}

export default Layout