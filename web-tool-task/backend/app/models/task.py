from sqlalchemy import Column, String, DateTime, Enum, ForeignKey, JSON
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.core.database import Base
import uuid

class Task(Base):
    __tablename__ = "tasks"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    task_key = Column(String(100), unique=True, nullable=False)
    name = Column(String(500), nullable=False)
    description = Column(String(2000))
    status = Column(String(50), default="DRAFT")
    priority = Column(String(20), default="MEDIUM")
    
    # 4 IDs liên kết
    jira_id = Column(UUID(as_uuid=True), nullable=True)
    git_id = Column(UUID(as_uuid=True), nullable=True)
    sharepoint_id = Column(UUID(as_uuid=True), nullable=True)
    confluence_id = Column(UUID(as_uuid=True), nullable=True)
    
    created_by = Column(UUID(as_uuid=True), nullable=True)
    created_at = Column(DateTime(timezone=True), server_default=func.now())
    updated_at = Column(DateTime(timezone=True), onupdate=func.now())
    executed_at = Column(DateTime(timezone=True), nullable=True)