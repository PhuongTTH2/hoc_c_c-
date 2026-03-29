from sqlalchemy import Column, String, DateTime, JSON, Integer
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.core.database import Base
import uuid

class GitDetails(Base):
    __tablename__ = "git_details"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    branch_name = Column(String(255), nullable=False)
    branch_url = Column(String(500))
    repo_url = Column(String(500))
    repo_name = Column(String(255))
    base_branch = Column(String(100), default="main")
    provider = Column(String(50), default="GITHUB")
    commit_sha = Column(String(100))
    raw_response = Column(JSON, default=dict)
    created_at = Column(DateTime(timezone=True), server_default=func.now())