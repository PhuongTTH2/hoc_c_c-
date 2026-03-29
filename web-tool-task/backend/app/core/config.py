from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    DATABASE_URL: str = "postgresql://webuser:webpass123@postgres:5432/web_tool_db"
    REDIS_URL: str = "redis://redis:6379/0"
    JWT_SECRET: str = "dda4c80f915f62834c3ee47907f1ac757798786470300bce6f09acd1134bb81d"
    JWT_ALGORITHM: str = "HS256"
    JWT_EXPIRES_MINUTES: int = 60 * 24 * 7  # 7 days

    class Config:
        env_file = ".env"

settings = Settings()