# app/core/config.py
from pydantic_settings import BaseSettings
from typing import Optional

class Settings(BaseSettings):
    # Database
    DATABASE_URL: str = "postgresql://webuser:webpass123@postgres:5432/web_tool_db"
    
    # JWT
    JWT_SECRET: str = "your-secret-key-change-in-production"
    JWT_ALGORITHM: str = "HS256"
    JWT_EXPIRES_MINUTES: int = 60 * 24  # 24 hours
    
    # Jira
    JIRA_URL: Optional[str] = None
    JIRA_TOKEN: Optional[str] = None
    
    # Git
    GIT_TOKEN: Optional[str] = None
    
    # SharePoint
    SHAREPOINT_CLIENT_ID: Optional[str] = None
    SHAREPOINT_CLIENT_SECRET: Optional[str] = None
    
    # Confluence
    CONFLUENCE_URL: Optional[str] = None
    CONFLUENCE_TOKEN: Optional[str] = None
    
    class Config:
        env_file = ".env"

settings = Settings()